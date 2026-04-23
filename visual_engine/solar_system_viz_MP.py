# data handlers
import pandas as pd
import numpy as np

# plotting and renderingWE
import matplotlib
import matplotlib.pyplot as plt
matplotlib.use('Agg')
import sequential_renderer

# multiprocessing and time logging/file handling
import multiprocessing
import time
import os
import glob

# beautification modules
from console_colors import bcolors
from alive_progress import alive_bar

np.random.seed(19680801)

# --- CONFIGURATION ---
frames_to_render = 20000000
frame_step = 40  # Reduced step for smoother visualization, adjust as needed
data_dir = r'C:/Users/Public/Public Misc/OrbitForge-Engine/simulation_data/'
frames_output_path = r'C:/Users/Public/Public Misc/OrbitForge-Engine/orbit_frames/'
anim_output_path = r'C:/Users/Public/Public Misc/OrbitForge-Engine/'

anim_title = 'Mission_Playback'
anim_extension = '.mp4'
anim_fps = 24
anim_dpi = 100

lock_frame_to_pgb = True
space_bounds_x = 2 # Adjusted for inner solar system (AU)
space_bounds_y = 2

theme_font_family = 'monospace'
theme_font_color = 'white'
theme_font_weight = 'normal'
theme_font_size = 15

plt.style.use('dark_background')
plt.rcParams.update({
    'font.family': theme_font_family,
    'font.size': theme_font_size,
    'font.weight': theme_font_weight,
    'text.color': theme_font_color,
    'axes.labelcolor': theme_font_color,
    'xtick.color': theme_font_color,
    'ytick.color': theme_font_color
})

# Globals for workers
worker_assigned_axis = None
worker_assigned_fig = None

def worker_task():
    global worker_assigned_axis, worker_assigned_fig
    worker_assigned_fig = plt.figure(figsize=(10, 10), facecolor="#000000")
    worker_assigned_fig.tight_layout(pad=3.0)
    worker_assigned_axis = worker_assigned_fig.add_subplot()
    worker_assigned_axis.set_facecolor("#000000")

def style_worker_axes(ax):
    ax.set_xlabel('X (AU)', color='#888888', fontsize=10, labelpad=10)
    ax.set_ylabel('Y (AU)', color='#888888', fontsize=10, labelpad=10)
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)
    ax.spines['bottom'].set_color("#888888") 
    ax.spines['bottom'].set_linewidth(1.5)
    ax.spines['left'].set_color("#888888")
    ax.spines['left'].set_linewidth(1.5)
    ax.minorticks_on()
    ax.tick_params(axis='both', which='major', colors="#888888", labelsize=8, direction='in', length=7, width=1.5, pad=8)
    ax.tick_params(axis='both', which='minor', colors="#888888", direction='in', length=4, width=1)
    ax.grid(True, which='major', color="#8DBBFF1A", linestyle='-', linewidth=0.2, zorder=0)
    ax.grid(True, which='minor', color="#8DBBFF1A", linestyle=':', linewidth=0.2, zorder=0)

def render_frame_task(args):
    global worker_assigned_fig, worker_assigned_axis
    frame, dyn_data, static_data, traj_data, manifest, bodies, colors = args

    try:
        frame_time = dyn_data['Time'][frame]
        pgb_x = dyn_data['Sun_X'][frame]
        pgb_y = dyn_data['Sun_Y'][frame]
        
        worker_assigned_axis.clear()
        style_worker_axes(worker_assigned_axis)

        if lock_frame_to_pgb:
            worker_assigned_axis.set_xlim(pgb_x - space_bounds_x, pgb_x + space_bounds_x)
            worker_assigned_axis.set_ylim(pgb_y - space_bounds_y, pgb_y + space_bounds_y)
        else:
            worker_assigned_axis.set_xlim(-space_bounds_x, space_bounds_x)
            worker_assigned_axis.set_ylim(-space_bounds_y, space_bounds_y)

        worker_assigned_axis.set_title(f'Mission: {manifest["Mission_ID"]}', loc='center', color='white', pad=15, fontweight='bold')

        mission_text = (
            r'$\mathbf{MISSION \ OVERVIEW}$' + '\n' +
            f'T+ {frame_time:06.2f} Days\n' +
            f'Origin: {manifest["Origin"]}\n' +
            f'Target: {manifest["Target"]}\n' +
            f'Launch Day: {manifest["Launch_Day"]}\n' +
            fr'Phase Angle $(\phi): {manifest["Launch_Phase_Angle"]:.2f}\degree$'
        )
        worker_assigned_axis.text(0.98, 0.97, mission_text, transform=worker_assigned_axis.transAxes,
                                  fontsize=9, verticalalignment='top', horizontalalignment='right',
                                  bbox=dict(facecolor='black', alpha=0.7, edgecolor='#444444', boxstyle='round,pad=0.5'))

        ship_id = manifest['Ship_ID']
        launch_day = manifest['Launch_Day']

        for body in bodies:
            if static_data[body]['draw'] == 0: continue
            
            # Hide ship before launch
            if body == ship_id and frame_time < launch_day: continue
            current_x = dyn_data[f'{body}_X'][frame]
            current_y = dyn_data[f'{body}_Y'][frame]
            shape = static_data[body]['shape']
            radius = static_data[body]['radius']
            color = colors[body]

            # trail logic
            if body == ship_id:
                # find index where time == launch day
                launch_idx = np.searchsorted(dyn_data['Time'], launch_day)
                trail_x = dyn_data[f'{body}_X'][launch_idx:frame+1]
                trail_y = dyn_data[f'{body}_Y'][launch_idx:frame+1]
                worker_assigned_axis.plot(trail_x, trail_y, color=color, ls='-', linewidth=1.5, alpha=0.9)
            else:
                trail_x = dyn_data[f'{body}_X'][0:frame+1]
                trail_y = dyn_data[f'{body}_Y'][0:frame+1]
                worker_assigned_axis.plot(trail_x, trail_y, color=color, ls='--', linewidth=1, alpha=0.5)

            # current body positiojn
            worker_assigned_axis.plot(current_x, current_y, label=body, marker=shape, ls='', color=color, ms=radius)

            # telemetry HUD
            if body in [manifest['Origin'], manifest['Target']]:
                true_anom = np.rad2deg(dyn_data[f'{body}_true_anomaly'][frame])
                ecc_anom = np.rad2deg(dyn_data[f'{body}_eccentric_anomaly'][frame])
                
                hud_text = (
                    fr'$\mathbf{{{body.upper()} \ TELEMETRY}}$' + '\n' +
                    rf'True Anom ($\theta$): ${true_anom:06.2f}^\circ$' + '\n' +
                    rf'Ecc  Anom ($\phi$): ${ecc_anom:06.2f}^\circ$'
                )
                
                # origin HUD on Left, target HUD on Bottom
                loc = (0.03, 0.97) if body == manifest['Origin'] else (0.03, 0.03)
                va = 'top' if body == manifest['Origin'] else 'bottom'
                
                worker_assigned_axis.text(loc[0], loc[1], hud_text, transform=worker_assigned_axis.transAxes,
                                          fontsize=9, verticalalignment=va, horizontalalignment='left',
                                          bbox=dict(facecolor='black', alpha=0.6, edgecolor=color, boxstyle='round,pad=0.5'))

        # ghost trajectory for rocket
        if frame_time >= launch_day:
            traj_x = traj_data['Traj_X'][:]
            traj_y = traj_data['Traj_Y'][:]
            worker_assigned_axis.plot(traj_x, traj_y, label='Targeting Solution', ls=':', color=colors[ship_id], lw=1.2, alpha=0.8)

        # Legend
        if not worker_assigned_axis.get_legend():
            worker_assigned_axis.legend(fontsize=9, markerscale=0.8, loc='center right', framealpha=0.3, edgecolor='#444444', labelcolor='white')

        worker_assigned_fig.savefig(frames_output_path+fr'/frame{frame:04d}.png', dpi=anim_dpi, pad_inches=0.2, facecolor=worker_assigned_fig.get_facecolor())
        return (frame, True)

    except Exception as e:
        print(f'{bcolors.FAIL}Failed to render frame {frame}.\nCause: {e}{bcolors.ENDC}')
        return (frame, False)
    
if __name__=='__main__':
    os.makedirs(frames_output_path, exist_ok=True)
    print(f"{bcolors.BOLD}Clearing old frames...{bcolors.ENDC}")
    for file_name in os.listdir(frames_output_path):
        os.remove(os.path.join(frames_output_path, file_name))

    try:
        # Load the dynamic and static data
        dyn_df = pd.read_csv(data_dir + "simulation_dynamic_data.csv")
        static_df = pd.read_csv(data_dir + "simulation_static_data.csv")
        traj_df = pd.read_csv(data_dir + "rocket_traj_data.csv")
        
        # Dynamically find and load the manifest (assuming only one exists in the dir)
        manifest_files = glob.glob(data_dir + "*_manifest.csv")
        manifest_df = pd.read_csv(manifest_files[0])
        manifest = manifest_df.iloc[0].to_dict() # Convert first row to dictionary
        
    except Exception as e:
        print(f'{bcolors.FAIL}Failed to read simulation data...\nCause: {e}{bcolors.ENDC}')
        exit()
    
    print(f'{bcolors.OKGREEN}Successfully loaded data for Mission: {manifest["Mission_ID"]}{bcolors.ENDC}')

    # process Static Data into a fast dictionary lookup
    static_data = {}
    for _, row in static_df.iterrows():
        # Clean up shapes (matplotlib gets mad if shapes are read as strings instead of characters)
        shape_str = str(row['body_shape']).strip()
        static_data[row['body_name']] = {
            'mass': row['body_mass'],
            'radius': row['body_radius'],
            'shape': shape_str if shape_str else 'o',
            'draw': int(row['body_draw'])
        }

    # Convert dynamic dataframe to fast numpy dictionary
    np_dyn_data = {col: dyn_df[col].values for col in dyn_df.columns}
    np_traj_data = {col: traj_df[col].values for col in traj_df.columns}
    
    celestial_bodies = list(static_data.keys())

    # assign random colors to all, then set special colors for specific bodies
    colors = {body: np.random.rand(3) for body in celestial_bodies}
    color_map = {'Sun': 'yellow', 'Mercury': 'darkgray', 'Venus': 'gold', 'Earth': 'royalblue', 'Mars': 'red', 'Jupiter':'tan'}
    for name, c in color_map.items():
        if name in colors: colors[name] = c
    colors[manifest['Ship_ID']] = 'papayawhip'

    print(f"\n{bcolors.UNDERLINE}Starting Renderer...{bcolors.ENDC}")
    core_count = multiprocessing.cpu_count()
    worker_count = max(1, core_count - 1)

    choice_render = input(f'\n{bcolors.WARNING}Start rendering? (Y/N)\n--> {bcolors.ENDC}')

    if choice_render.lower() == 'y':
        render_time_start = time.time()
        frames_to_render = min(frames_to_render, len(dyn_df))
        frames = range(1, frames_to_render, frame_step)

        task_args = [(frame, np_dyn_data, static_data, np_traj_data, manifest, celestial_bodies, colors) for frame in frames]
        
        frame_success, frame_failed = 0, 0
        with multiprocessing.Pool(processes=worker_count, initializer=worker_task) as pool:
            with alive_bar(len(frames)) as bar:
                for frame, success in pool.imap_unordered(render_frame_task, task_args, chunksize=4):
                    if success: frame_success += 1
                    else: frame_failed += 1
                    bar() 

        render_time = time.time() - render_time_start
        print(f'\n{bcolors.OKGREEN}Finished in {render_time:.2f}s. Success: {frame_success} | Failed: {frame_failed}{bcolors.ENDC}')
        
        # pass frame sequence to sequential renderer
        sequential_renderer.generate_video(frames_output_path, anim_fps, anim_title+anim_extension, anim_output_path)