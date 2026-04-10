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

# animation duration parameters 
frames_to_render=1500000;
frame_step=1000;
frames_output_path = r'C:/Users/lenovo/Documents/Lamberts_BVP/orbit_frames/'

# animation file parameters
anim_title='Solar System'
anim_extension='.mp4'
anim_fps=10
anim_dpi=75
anim_output_path = r'C:/Users/lenovo/Documents/Lamberts_BVP/'

# frames

lock_frame_to_pgb=True

space_bounds_x=2.5
space_bounds_y=2.5

theme_font_family = 'Segoe UI'
theme_font_color = 'white'
theme_font_weight = 'normal'
theme_font_size = 6

plt.style.use('dark_background')
plt.rcParams.update( {
    'font.family':theme_font_family,
    'font.size':theme_font_size,
    'font.weight':theme_font_weight,
    'text.color':theme_font_color,
    'axes.labelcolor':theme_font_color,
    'xtick.color':theme_font_color,
    'ytick.color':theme_font_color
})
#plt.rcParams['text.usetex']=True

# worker task: what each worker does beforehand
worker_assigned_axis = None
worker_assigned_fig = None
def worker_task():
    global worker_assigned_axis, worker_assigned_fig
    
    # individual figures and axes
    worker_assigned_fig = plt.figure()
    worker_assigned_fig.tight_layout()
    worker_assigned_axis = worker_assigned_fig.add_subplot()

def style_worker_axes(ax):
    # set parameters and visuals
    ax.set_xlabel('X (AU)')
    ax.set_ylabel('Y (AU)')

    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)

    # Dim the bottom and left borders
    ax.spines['bottom'].set_color('#444444')
    ax.spines['left'].set_color('#444444')
    
    # Make tick marks subtle
    ax.tick_params(colors='#888888', labelsize=10)
    
    # Push the grid to the absolute background (zorder=0) and make it very faint
    ax.grid(True, color="#8DBBFF92", linestyle='-', linewidth=0.25, zorder=0)

# frame rendering task: what each worker does as to 'render' each frame
def render_frame_task(args):
    global worker_assigned_fig, worker_assigned_axis
    # get frame number, simulation data and other things via multiprocessing argument
    frame, ss_data, rocket_data, rocket_traj_data, celestial_bodies, bodies_colors, rocket_color = args

    try:
        # set parameters and visuals 
        pgb_x = ss_data['Sun_X'][frame]
        pgb_y = ss_data['Sun_Y'][frame]

        frame_time = ss_data['Time'][frame]
        
        worker_assigned_axis.clear()

        style_worker_axes(worker_assigned_axis)

        if lock_frame_to_pgb:
            worker_assigned_axis.set_xlim(pgb_x-space_bounds_x, pgb_x+space_bounds_x)
            worker_assigned_axis.set_ylim(pgb_y-space_bounds_y, pgb_y+space_bounds_y)
        else:
            worker_assigned_axis.set_xlim(-space_bounds_x, space_bounds_x)
            worker_assigned_axis.set_ylim(-space_bounds_y, space_bounds_y)


        worker_assigned_axis.set_title(f'N-Body Orbital Simulation\nt: {frame_time:06.2f} days')
        worker_assigned_axis.grid(True, linestyle='--', alpha=0.4)

        for body in celestial_bodies:
            # trail data upto this point
            if ss_data[f'{body}_draw'][1]==True:
                trail_start = max(0, frame-1000)
                trail_x = ss_data[f'{body}_X'][0:frame+1]
                trail_y = ss_data[f'{body}_Y'][0:frame+1]
                worker_assigned_axis.plot(trail_x, trail_y, color=bodies_colors[body], ls='--', linewidth=1, alpha=0.6)

                # plot current point with circle (scatterplot)
                current_x=ss_data[f'{body}_X'][frame]
                current_y=ss_data[f'{body}_Y'][frame]
    
                body_shape=ss_data[f'{body}_shape'][frame]  
                body_radius=ss_data[f'{body}_radius'][frame]

                worker_assigned_axis.plot(current_x, current_y, label=body, marker=body_shape, ls='', color=bodies_colors[body], ms=body_radius)

                if body=='Earth':
                    true_anomaly=np.rad2deg(ss_data['Earth_true_anomaly'][frame])
                    ecc_anomaly=np.rad2deg(ss_data['Earth_eccentric_anomaly'][frame])

                    x_E = ss_data[f'Earth_XfromE'][frame]
                    y_E = ss_data[f'Earth_YfromE'][frame]

                    worker_assigned_axis.text(-2.25, 2, rf'Earth True Anomaly ($\theta_1$): ${true_anomaly:.2f}\degree$', bbox=dict(facecolor='white', alpha=0.1))
                    worker_assigned_axis.text(-2.25, 1.5, rf'Earth Eccentric Anomaly ($\phi_1$): ${ecc_anomaly:.2f}\degree$', bbox=dict(facecolor='white', alpha=0.1))
                    worker_assigned_axis.text(-2.25, 1.0, rf'Earth X: ${current_x}$', bbox=dict(facecolor='white', alpha=0.1))
                    worker_assigned_axis.text(-2.25, 0.5, rf'Earth Y: ${current_y}$', bbox=dict(facecolor='white', alpha=0.1))
                    worker_assigned_axis.text(-2.25, 0, rf'Earth X_E: ${x_E}$', bbox=dict(facecolor='white', alpha=0.1))
                    worker_assigned_axis.text(-2.25, -.5, rf'Earth X_E: ${y_E}$', bbox=dict(facecolor='white', alpha=0.1))

                if body=='Mars':
                    true_anomaly=np.rad2deg(ss_data['Mars_true_anomaly'][frame])
                    ecc_anomaly=np.rad2deg(ss_data['Mars_eccentric_anomaly'][frame])

                    worker_assigned_axis.text(-2.25, -1, rf'Mars True Anomaly ($\theta_2$): ${true_anomaly:.2f}\degree$', bbox=dict(facecolor='white', alpha=0.1))
                    worker_assigned_axis.text(-2.25, -1.5, rf'Mars Eccentric Anomaly ($\phi_2$): ${ecc_anomaly:.2f}\degree$', bbox=dict(facecolor='white', alpha=0.1))

        if (frame_time > 20):
            trail_x = rocket_data['Rocket_X'][0:frame+1]
            trail_y = rocket_data['Rocket_Y'][0:frame+1]
            worker_assigned_axis.plot(trail_x, trail_y, color=rocket_color, ls='dashdot', linewidth=1, alpha=0.8)

            current_x = rocket_data['Rocket_X'][frame]
            current_y = rocket_data['Rocket_Y'][frame]
            worker_assigned_axis.plot(current_x, current_y, label='Rocket', marker='^', ls='', color=rocket_color, ms=1)

            traj_x = rocket_traj_data['Traj_X']
            traj_y = rocket_traj_data['Traj_Y']
            worker_assigned_axis.plot(traj_x, traj_y, label='Trajectory', ls='--', color='white', lw=.1, alpha=1)


        if not worker_assigned_axis.get_legend():
            worker_assigned_axis.legend(fontsize='medium', markerscale=0.5, loc='upper right', framealpha=0.5, edgecolor='white', labelcolor='white')

        # save figure to disk 
        worker_assigned_fig.savefig(frames_output_path+fr'/frame{frame:04d}.png', 
                                    dpi=anim_dpi,
                                    pad_inches=0.2,
                                    facecolor=worker_assigned_fig.get_facecolor())
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
        ss_data = pd.read_csv("C:/Users/lenovo/Documents/Lamberts_BVP/simulation_data/solar_system_data.csv")
        rocket_data = pd.read_csv("C:/Users/lenovo/Documents/Lamberts_BVP/simulation_data/rocket_data.csv")
        rocket_traj_data = pd.read_csv("C:/Users/lenovo/Documents/Lamberts_BVP/simulation_data/rocket_traj_data.csv")
        
    except Exception as e:
        print(f'{bcolors.FAIL}Failed to read simulation data...\nCause: {e}')
    else:
        print(f'{bcolors.OKGREEN}Successfully read simulation data!')

    np_ss_data={col:ss_data[col].values for col in ss_data.columns}
    np_rocket_data={col:rocket_data[col].values for col in rocket_data.columns}
    np_rocket_traj_data={col:rocket_traj_data[col].values for col in rocket_traj_data.columns}

    # load celestial bodies names from data and assign random colors to each (dictionary)
    celestial_bodies = [col.replace('_X','') for col in ss_data.columns if col.endswith('_X')]
    rocket = [col.replace('_X','') for col in rocket_data.columns if col.endswith('_X')][0]

    # give all bodies a random color. especially meant for miscellaneous bodies such as asteroids
    bodies_colors={body:np.random.rand(3) for body in celestial_bodies}
    rocket_color=np.random.rand(3)

    # now give special colors to some special bodies
    bodies_colors['Sun']='yellow'
    bodies_colors['Mercury']='darkgray'
    bodies_colors['Venus']='gold'
    bodies_colors['Earth']='royalblue'
    bodies_colors['Mars']='red'
    # not accounting outer belt planets for now as they have minimal input of gravity 

    #bodies_colors['Jupiter']='lemonchiffon'
    #bodies_colors['Saturn']='beige'
    #bodies_colors['Uranus']='lightcyan'
    #bodies_colors['Neptune']='mediumslateblue'

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

    choice_render = input(f'\n{bcolors.WARNING}Start rendering? (Y/N)\n--> {bcolors.ENDC}')

    if choice_render.lower()=='y':
        print(f'{bcolors.BOLD}\n\n-------STARTING PARALLEL PROCESSING ON {core_count} CORES-------------')

        render_time_start=time.time()
        frames_to_render=min(frames_to_render, len(ss_data))
        frames = range(1, frames_to_render, frame_step)

        # arguments to be passed to each worker task
        task_args=[(frame, np_ss_data, np_rocket_data, np_rocket_traj_data, celestial_bodies, bodies_colors, rocket_color) for frame in frames]
        
        # start multiprocessing
        with multiprocessing.Pool(processes=worker_count, initializer=worker_task) as pool:
            # use alive bar for beautification (awesome module)
            with alive_bar(len(frames)) as bar:
                for frame, success in pool.imap_unordered(render_frame_task, task_args, chunksize=4):
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
        sequential_renderer.generate_video(frames_output_path, anim_fps, anim_title+anim_extension, anim_output_path)

    else:
        print(f'{bcolors.WARNING}Cancelled render...')
