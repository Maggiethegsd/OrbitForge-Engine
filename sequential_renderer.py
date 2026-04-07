import os 
import cv2
from natsort import natsorted 
from console_colors import bcolors

from alive_progress import alive_bar

def generate_video(sequence_path, framerate, video_title, anim_render_path):
    os.chdir(sequence_path)
    
    image_count = len([file for file in os.listdir('.') if file.endswith('.png')])
    # print(f'No. of images: {image_count}')
            
    # video gen
    images = [img for img in os.listdir(sequence_path) if img.endswith('.png')]
    images = natsorted(images, key=str.lower)
    # print("Images: ", images)
    
    frame = cv2.imread(os.path.join(sequence_path, images[0]))
    height, width, layers = frame.shape
    
    print(f'{bcolors.OKGREEN}Generating video...{bcolors.ENDC}')
    
    # Video writer to create .avi file
    video = cv2.VideoWriter(video_title, cv2.VideoWriter_fourcc(*'mp4v'), fps=framerate, frameSize=(width, height))
    # Appending images to video
    with alive_bar(len(images)) as bar:
        for image in images:
            try:
                video.write(cv2.imread(os.path.join(sequence_path, image)))
            except Exception as e:
                print(e)
            
            bar()
            
        # Release the video file
    try: 
        video.release()
        cv2.destroyAllWindows()
        print(f"{bcolors.OKGREEN}Video generated successfully!{bcolors.ENDC}")
        os.startfile(sequence_path+video_title)
    except:
        print(f"{bcolors.FAIL}Failed to generate video.{bcolors.ENDC}")
        
        
