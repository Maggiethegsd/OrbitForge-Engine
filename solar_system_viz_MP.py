# data handlers
import pandas as pd
import numpy as np

# plotting and rendering
import matplotlib
import matplotlib.pyplot as plt
matplotlib.use('Agg')
import sequential_renderer

# multiprocessing and time logging/file handling
import multiprocessing
import time
import os

# beautification modules
from console_colors import bcolors
from alive_progress import alive_bar

# random file seed for reproducibility
np.random.seed(19680801)

# animation parameters 
frames_to_render=80000
frame_step=50
frames_output_path = r'C:/Users/lenovo/Documents/Lamberts Problem/orbit_frames/'

# note - animation fps will be scaled by frame stepping to interpolate accurate time
anim_title='Solar System'
anim_extension='.mp4'
anim_fps=90
anim_dpi=75
anim_output_path = r'C:/Users/lenovo/Documents/Lamberts Problem/'

lock_frame_to_pgb=True

space_bounds_x=700
space_bounds_y=700

plt.style.use('dark_background')

# worker task: what each worker does beforehand
worker_assigned_axis = None
worker_assigned_fig = None
def worker_task():
    global worker_assigned_axis, worker_assigned_fig
    
    # individual figures and axes
    worker_assigned_fig = plt.figure()
    worker_assigned_axis = worker_assigned_fig.add_subplot()

def style_worker_axes(ax):
    # set parameters and visuals
    ax.set_xlabel('X')
    ax.set_ylabel('Y')

# frame rendering task: what each worker does as to 'render' each frame
def render_frame_task(args):
    global worker_assigned_fig, worker_assigned_axis
    # get frame number, simulation data and other things via multiprocessing argument
    frame, data, celestial_bodies, bodies_colors = args

    try:
        # set parameters and visuals 
        pgb_x = data['Sun_X'][frame]
        pgb_y = data['Sun_Y'][frame]
        
        worker_assigned_axis.clear()

        style_worker_axes(worker_assigned_axis)
        worker_assigned_axis.set_xlim(pgb_x-space_bounds_x, pgb_x+space_bounds_x)
        worker_assigned_axis.set_ylim(pgb_y-space_bounds_y, pgb_y+space_bounds_y)

        worker_assigned_axis.set_title(f'N-Body Orbital Simulation\nTime: {round(data['Time'][frame],2)}s')
        worker_assigned_axis.grid(True, linestyle='--', alpha=0.4)

        for body in celestial_bodies:
            # trail data upto this point
            trail_start=max(0, frame-500)
            trail_x = data[f'{body}_X'][0:frame+1]
            trail_y = data[f'{body}_Y'][0:frame+1]
            worker_assigned_axis.plot(trail_x, trail_y, color=bodies_colors[body], ls='--', linewidth=1, alpha=0.6)

            # plot current point with circle (scatterplot)
            current_x=data[f'{body}_X'][frame]
            current_y=data[f'{body}_Y'][frame]

            body_shape=data[f'{body}_shape'][frame]
            body_radius=data[f'{body}_radius'][frame]

            worker_assigned_axis.plot(current_x, current_y, label=body, marker=body_shape, ls='', color=bodies_colors[body], ms=body_radius, mec='white')
        
        if not worker_assigned_axis.get_legend():
            worker_assigned_axis.legend(fontsize='medium', markerscale=0.5, loc='upper right', framealpha=0.5, edgecolor='white', labelcolor='white')

        # save figure to disk 
        worker_assigned_fig.savefig(frames_output_path+fr'/frame{frame:04d}.png', dpi=anim_dpi)
        return (frame, True)
    
    except Exception as e:
        print(f'{bcolors.FAIL}Failed to render frame {frame}.\n{bcolors.OKCYAN}Cause:{bcolors.FAIL}{e}')
        return (frame, False)
    
# central thread
if __name__=='__main__':
    # replace directory if its already made
    os.makedirs(frames_output_path, exist_ok=True)

    # clear out old frames to avoid overlapping error I faced
    print(f"{bcolors.BOLD}Clearing old frames...{bcolors.ENDC}")

    for file_name in os.listdir(frames_output_path):
        file_path = os.path.join(frames_output_path, file_name)
        if os.path.isfile(file_path):
            os.remove(file_path)  

    # load csv data into pandas dataframe
    try:
        data = pd.read_csv("solar_system_data.csv")
    except Exception as e:
        print(f'{bcolors.FAIL}Failed to read simulation data...\nCause: {e}')
    else:
        print(f'{bcolors.OKGREEN}Successfully read simulation data!')

    np_data={col:data[col].values for col in data.columns}

    # load celestial bodies names from data and assign random colors to each (dictionary)
    celestial_bodies = [col.replace('_X','') for col in data.columns if col.endswith('_X')]
    bodies_colors={body:np.random.rand(3) for body in celestial_bodies}
    bodies_colors['Earth']='blue'
    bodies_colors['Sun']='yellow'
    bodies_colors['Mars']='red'

    frame_success=0
    frame_failed=0
    if frame_step<=0:
        frame_step=1

    print(f"\n{bcolors.UNDERLINE}Loading simulation data...{bcolors.ENDC}")
    
    print(f'{bcolors.HEADER}\n\n------RENDER PROPERTIES------{bcolors.ENDC}')
    print(f'{bcolors.OKCYAN}\nFile Name: {anim_title} {bcolors.ENDC}')
    print(f'{bcolors.OKCYAN}File Extension: {anim_extension} {bcolors.ENDC}')
    print(f'{bcolors.OKCYAN}Frame Rate: {anim_fps}fps {bcolors.ENDC}')
    print(f'{bcolors.OKCYAN}DPI: {anim_dpi} {bcolors.ENDC}')

    core_count=multiprocessing.cpu_count()

    worker_count=max(1, core_count-1) if core_count > 4 else core_count

    print(f'{bcolors.OKCYAN}Cores Available: {core_count}')
    print(f'{bcolors.OKCYAN}Workers to be used: {worker_count}')

    choice_render = input(f'\n{bcolors.WARNING}Start rendering? (Y/N){bcolors.ENDC}')

    if choice_render.lower()=='y':
        print(f'{bcolors.BOLD}\n\n-------STARTING PARALLEL PROCESSING ON {core_count} CORES-------------')

        render_time_start=time.time()
        frames_to_render=max(frames_to_render, len(data))
        frames = range(1, frames_to_render+1, frame_step)

        # arguments to be passed to each worker task
        task_args=[(frame, np_data, celestial_bodies, bodies_colors) for frame in frames]
        
        # start multiprocessing
        with multiprocessing.Pool(processes=worker_count, initializer=worker_task) as pool:
            # use alive bar for beautification (awesome module)
            with alive_bar(len(frames)) as bar:
                for frame, success in pool.imap_unordered(render_frame_task, task_args):
                    if success:
                        frame_success+=1
                    else:
                        frame_failed+=1
                        print(f'{bcolors.FAIL}\nCould not render frame {frame}.{bcolors.ENDC}')
                        break
                    bar() 


        render_time_end=time.time()
        render_time=render_time_end-render_time_start

        print(f'\nRender complete. Finished in {render_time:.2f}s.')
        print(f'\n{bcolors.OKGREEN}SUCCESSFUL FRAMES RENDERED: {frame_success}{bcolors.ENDC}')
        print(f'\n{bcolors.FAIL}FRAMES FAILED: {frame_failed}{bcolors.ENDC}')

        print(f'{bcolors.OKBLUE}Finished Rendering Frames!{bcolors.ENDC}')

        # pass frame sequence to sequential renderer
        sequential_renderer.generate_video(frames_output_path, anim_fps*10/frame_step, anim_title+anim_extension, anim_output_path)

    else:
        print(f'{bcolors.WARNING}Cancelled render...')
