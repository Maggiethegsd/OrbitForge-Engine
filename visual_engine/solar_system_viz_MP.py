import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.widgets as widgets
import os
import glob
import time

# beautification modules
from console_colors import bcolors
np.random.seed(19680801)

# --- CONFIGURATION ---
data_dir = os.getcwd() + "/simulation_data/"

lock_frame_to_pgb = True
space_bounds_x = 2 # Adjusted for inner solar system (AU)
space_bounds_y = 2

theme_font_family = 'monospace'
theme_font_color = 'white'
theme_font_weight = 'normal'
theme_font_size = 12

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

# --- DATA LOADING ---
try:
    dyn_df = pd.read_csv(data_dir + "simulation_dynamic_data.csv")
    static_df = pd.read_csv(data_dir + "simulation_static_data.csv")
    traj_df = pd.read_csv(data_dir + "rocket_traj_data.csv")
    
    manifest_files = glob.glob(data_dir + "*_manifest.csv")
    manifest_df = pd.read_csv(manifest_files[0])
    manifest = manifest_df.iloc[0].to_dict()
except Exception as e:
    print(f'{bcolors.FAIL}Failed to read simulation data...\nCause: {e}{bcolors.ENDC}')
    exit()

print(f'{bcolors.OKGREEN}Successfully loaded data for Mission: {manifest["Mission_ID"]}{bcolors.ENDC}')

# Process static data
static_data = {}
for _, row in static_df.iterrows():
    shape_str = str(row['body_shape']).strip()
    static_data[row['body_name']] = {
        'mass': row['body_mass'],
        'radius': row['body_radius'],
        'shape': shape_str if shape_str else 'o',
        'draw': int(row['body_draw'])
    }

celestial_bodies = list(static_data.keys())

# Setup colors
colors = {body: np.random.rand(3) for body in celestial_bodies}
color_map = {'Sun': 'yellow', 'Mercury': 'darkgray', 'Venus': 'gold', 'Earth': 'royalblue', 'Mars': 'red', 'Jupiter':'tan'}
for name, c in color_map.items():
    if name in colors: colors[name] = c
colors[manifest['Ship_ID']] = 'papayawhip'

# Extract fast arrays
time_array = dyn_df['Time'].values
max_frames = len(time_array) - 1
launch_day = manifest['Launch_Day']
ship_id = manifest['Ship_ID']
launch_idx = np.searchsorted(time_array, launch_day)

# --- SETUP INTERACTIVE PLOT ---
plt.ion() # Enable interactive mode
fig = plt.figure(figsize=(12, 10))
# Adjust bottom to make room for sliders
fig.subplots_adjust(bottom=0.25, left=0.1, right=0.9, top=0.9)
ax = fig.add_subplot()

# Style axes
ax.set_xlabel('X (AU)', color='#888888', fontsize=10, labelpad=10)
ax.set_ylabel('Y (AU)', color='#888888', fontsize=10, labelpad=10)
ax.spines['top'].set_visible(False)
ax.spines['right'].set_visible(False)
ax.spines['bottom'].set_color("#888888") 
ax.spines['left'].set_color("#888888")
ax.grid(True, which='major', color="#8DBBFF94", linestyle='-', linewidth=0.2, zorder=0)

ax.set_title(f'Mission: {manifest["Mission_ID"]}', loc='center', color='white', pad=15, fontweight='bold')

# --- INITIALIZE PLOT OBJECTS ---
# We store lines and points in dictionaries so we can update them rapidly without clearing the screen
plot_objects = {}

for body in celestial_bodies:
    if static_data[body]['draw'] == 0: continue
    c = colors[body]
    marker = static_data[body]['shape']
    ms = static_data[body]['radius']
    
    # Initialize empty trails and points
    trail_ls = '-' if body == ship_id else '--'
    trail_alpha = 0.9 if body == ship_id else 0.5
    
    trail_line, = ax.plot([], [], color=c, ls=trail_ls, linewidth=1.5, alpha=trail_alpha)
    point_marker, = ax.plot([], [], label=body, marker=marker, ls='', color=c, ms=ms)
    
    plot_objects[body] = {'trail': trail_line, 'point': point_marker}

# Ghost Trajectory Line
ghost_traj, = ax.plot([], [], label='Targeting Solution', ls=':', color=colors[ship_id], lw=1.2, alpha=0.8)

# HUD Texts
mission_hud = ax.text(0.98, 0.97, "", transform=ax.transAxes, fontsize=9, va='top', ha='right', bbox=dict(facecolor='black', alpha=0.7, edgecolor='#444444', boxstyle='round,pad=0.5'))
origin_hud = ax.text(0.03, 0.97, "", transform=ax.transAxes, fontsize=9, va='top', ha='left', bbox=dict(facecolor='black', alpha=0.6, edgecolor=colors[manifest['Origin']], boxstyle='round,pad=0.5'))
target_hud = ax.text(0.03, 0.03, "", transform=ax.transAxes, fontsize=9, va='bottom', ha='left', bbox=dict(facecolor='black', alpha=0.6, edgecolor=colors[manifest['Target']], boxstyle='round,pad=0.5'))

ax.legend(fontsize=9, markerscale=0.8, loc='center right', framealpha=0.3, edgecolor='#444444', labelcolor='white')

# --- INTERACTIVE WIDGETS ---
interface_color = "#46B5FFEF"
slider_props = dict(color=interface_color, track_color='#1a1a1a', initcolor='none', handle_style={'size': 10, 'facecolor': '#0a0a0a', 'edgecolor':interface_color})

ax_timeline = fig.add_axes([0.15, 0.15, 0.7, 0.03])
ax_speed = fig.add_axes([0.15, 0.10, 0.3, 0.03])
ax_zoom = fig.add_axes([0.15, 0.05, 0.3, 0.03]) # New Zoom Axis
ax_play = fig.add_axes([0.55, 0.075, 0.1, 0.04]) 

# Create the button, styling it to match the dark theme
play_button = widgets.Button(ax_play, 'Pause', color='#1a1a1a', hovercolor=interface_color)
play_button.label.set_color('white')
play_button.label.set_fontweight('bold')

timeline_slider = widgets.Slider(ax_timeline, 'Timeline', 0, max_frames, valinit=0, valstep=1, **slider_props)
speed_slider = widgets.Slider(ax_speed, 'Playback Speed', 0, 200, valinit=40, valstep=1, **slider_props)

zoom_slider = widgets.Slider(ax_zoom, 'Zoom (AU)', 0.05, 5.0, valinit=space_bounds_x, **slider_props) # New Zoom Slider

# Set the initial bounds ONCE, and keep orbits perfectly circular
ax.set_xlim(-space_bounds_x, space_bounds_x)
ax.set_ylim(-space_bounds_y, space_bounds_y)
ax.set_aspect('equal', adjustable='box') 

# Link the zoom slider to the plot limits
def update_zoom(val):
    bounds = zoom_slider.val
    # We get the current center so zooming doesn't snap you back to the sun if you panned away
    current_x_center = (ax.get_xlim()[0] + ax.get_xlim()[1]) / 2
    current_y_center = (ax.get_ylim()[0] + ax.get_ylim()[1]) / 2
    
    ax.set_xlim(current_x_center - bounds, current_x_center + bounds)
    ax.set_ylim(current_y_center - bounds, current_y_center + bounds)
    fig.canvas.draw_idle()

zoom_slider.on_changed(update_zoom)

# --- UPDATE LOGIC ---
def render_frame(frame):
    frame = int(frame)
    frame_time = time_array[frame]

    # Update Texts
    mission_hud.set_text(r'$\mathbf{MISSION \ OVERVIEW}$' + '\n' + f'T+ {frame_time:06.2f} Days\nOrigin: {manifest["Origin"]}\nTarget: {manifest["Target"]}\nLaunch Day: {launch_day}\n' + fr'Phase Angle $(\phi): {manifest["Launch_Phase_Angle"]:.2f}^\circ$')

    # Update Bodies
    for body in celestial_bodies:
        if static_data[body]['draw'] == 0: continue
        
        objs = plot_objects[body]
        # Hide ship before launch
        if body == ship_id and frame_time < launch_day:
            objs['point'].set_data([], [])
            objs['trail'].set_data([], [])
            continue
            
        current_x = dyn_df[f'{body}_X'].values[frame]
        current_y = dyn_df[f'{body}_Y'].values[frame]
        
        objs['point'].set_data([current_x], [current_y])
        
        if body == ship_id:
            objs['trail'].set_data(dyn_df[f'{body}_X'].values[launch_idx:frame+1], dyn_df[f'{body}_Y'].values[launch_idx:frame+1])
        else:
            objs['trail'].set_data(dyn_df[f'{body}_X'].values[0:frame+1], dyn_df[f'{body}_Y'].values[0:frame+1])

        # Telemetry HUDs
        if body in [manifest['Origin'], manifest['Target']]:
            true_anom = np.rad2deg(dyn_df[f'{body}_true_anomaly'].values[frame])
            ecc_anom = np.rad2deg(dyn_df[f'{body}_eccentric_anomaly'].values[frame])
            text = fr'$\mathbf{{{body.upper()} \ TELEMETRY}}$' + '\n' + rf'True Anom ($\theta$): ${true_anom:06.2f}^\circ$' + '\n' + rf'Ecc  Anom ($\phi$): ${ecc_anom:06.2f}^\circ$'
            if body == manifest['Origin']: origin_hud.set_text(text)
            else: target_hud.set_text(text)

    # Ghost Trajectory
    if frame_time >= launch_day:
        ghost_traj.set_data(traj_df['Traj_X'].values, traj_df['Traj_Y'].values)
    else:
        ghost_traj.set_data([], [])

    fig.canvas.draw_idle()

# Hook up slider event
timeline_slider.on_changed(render_frame)

# Initial draw
render_frame(0)
fig.canvas.draw()

# --- PLAYBACK LOOP ---
print(f"\n\n{bcolors.OKCYAN}Starting Live Playback...{bcolors.ENDC}\n\n")
is_playing = True

# New Button-driven logic
def toggle_play(event):
    global is_playing
    is_playing = not is_playing
    
    # Update button text based on state
    if is_playing:
        play_button.label.set_text('Pause')
    else:
        play_button.label.set_text('Play')
        
    fig.canvas.draw_idle()

# Hook the click event specifically to the button
play_button.on_clicked(toggle_play)


# Main loop
while plt.fignum_exists(fig.number):
    if is_playing:
        current_val = timeline_slider.val
        step = speed_slider.val
        
        new_val = current_val + step
        if new_val >= max_frames:
            new_val = max_frames
            is_playing = False # Auto-pause at end
            
        timeline_slider.set_val(new_val)
        
    fig.canvas.flush_events()
    time.sleep(0.01) # Keep UI responsive