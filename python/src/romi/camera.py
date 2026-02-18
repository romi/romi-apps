#
# Capture and store 10 images:
# python camera.py
#
# Display iamges without storing them to a file. Run indefinitly:
# python camera.py --show true --save false -count 0
#

import time
import websocket
from PIL import Image
from io import BytesIO
from rcom.rcom_client import RcomWSClient
import argparse
import cv2
import numpy as np

class Camera():

    @staticmethod
    def create(topic = 'camera', registry_ip = None):
        client = RcomWSClient(topic, topic, registry_ip)
        return Camera(client)
   
    def __init__(self, client):
        self.client = client
       
    def grab(self):
        cmd = f'{{"method": "camera:grab-jpeg-binary", "id": "{self.client.id}"}}'
        data = self.client.binary(cmd)
        print(f'data length {len(data)}')
        with open("tmp.jpg", "wb") as f:
            f.write(data)
        return Image.open(BytesIO(data))
       
    def start_recording(self):
        result = self.client.execute('camera:start-recording')
        return result['recording-id']
        
    def stop_recording(self, id):
        self.client.execute('camera:stop-recording', {'recording-id': id})
        
    def get_recording(self, id):
        result = self.client.execute('camera:get-recording', {'recording-id': id})
        return result['recording-path']

    def set_value(self, name, value):
        params = {'name': name, 'value': value}
        self.client.execute('camera:set-value', params)

    def select_option(self, name, value):
        params = {'name': name, 'value': value}
        self.client.execute('camera:select-option', params)
        
    def get_camera_info(self):
        return self.client.execute('camera:get-camera-info')
       
    def power_up(self):
        self.client.execute('power-up')
        
    def power_down(self):
        self.client.execute('power-down')


class FakeCamera():
    
    def __init__(self, filepath):
        self.image = Image.open(filepath)
       
    def grab(self):
        return self.image

    def set_value(self, name, value):
        params = {'name': name, 'value': value}
        print(f'camera:set-value: {name}={value}')

    def select_option(self, name, value):
        params = {'name': name, 'value': value}
        print(f'camera:select-option: {name}={value}')
       
    def power_up(self):
        #print(f'power-up')
        pass
        
    def power_down(self):
        #print(f'power-down')
        pass

    
def create_display_window(window_name="Image Window"):
    cv2.namedWindow(window_name, cv2.WINDOW_NORMAL)
    cv2.resizeWindow(window_name, 800, 600)
    return window_name


def show_image_in_window(image, window_name="Image Window", wait=1):
    np_img = np.array(image)
    if np_img.ndim == 3 and np_img.shape[2] == 3:
        img_to_show = cv2.cvtColor(np_img, cv2.COLOR_RGB2BGR)
    else:
        img_to_show = np_img
    cv2.imshow(window_name, img_to_show)
    cv2.waitKey(wait)


def blurriness(image):
    # compute the Laplacian of the image and then return the focus
    # measure, which is simply the variance of the Laplacian
    cv_image = np.array(image)[:, :, ::-1].copy()
    gray = cv2.cvtColor(cv_image, cv2.COLOR_BGR2GRAY)
    result = cv2.Laplacian(gray, cv2.CV_64F).var()
    return result


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--topic', type=str, nargs='?', default="camera",
                        help='The registry topic')
    parser.add_argument('--save', type=str, nargs='?', default="True",
                        help='Flag to save the files')
    parser.add_argument('--registry', type=str, nargs='?', default=None,
                        help='The IP address of the registry')
    parser.add_argument('--count', type=int, nargs='?', default=10,
                        help='The number of images')
    parser.add_argument('--sleep', type=float, nargs='?', default=0.0,
                        help='The delay between images')
    parser.add_argument('--show', type=bool, nargs='?', default=False,
                        help='Show the images')
    parser.add_argument('--blurriness', type=bool, nargs='?', default=False,
                        help='Print the blurriness index of the images')
    args = parser.parse_args()

    if args.show:
        create_display_window()
        
    camera = Camera.create(args.topic, args.registry)
    camera.power_up()

    if args.count == 0:
        args.count = 1000000

    n = 0
    while args.count == 0 or n < args.count:
        image = camera.grab()
        if image != None:
            if args.save:
                print(f"Saving {args.topic}-{n:05d}.jpg")
                image.save(f"{args.topic}-{n:05d}.jpg")
            if args.show:
                show_image_in_window(image)
            if args.blurriness:
                print(f"blurriness: {blurriness(image)}")
        time.sleep(args.sleep)
        n = n + 1
        
    camera.power_down()
