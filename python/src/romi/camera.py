import time
import websocket
from PIL import Image
from io import BytesIO
from rcom.rcom_client import RcomWSClient
import argparse

class Camera():
   
    def __init__(self, client, id="camera"):
        self.client = client
        self.id = id
       
    def grab(self):
        cmd = f'{{"method": "camera:grab-jpeg-binary", "id": "{self.id}"}}'
        data = self.client.binary(cmd)
        print(f'data length {len(data)}')
        with open("tmp.jpg", "wb") as f:
            f.write(data)
        return Image.open(BytesIO(data))

    def set_value(self, name, value):
        params = {'name': name, 'value': value}
        self.client.execute('camera:set-value', params)

    def select_option(self, name, value):
        params = {'name': name, 'value': value}
        self.client.execute('camera:select-option', params)
       
    def power_up(self):
        self.client.execute('power-up')
        
    def power_down(self):
        self.client.execute('power-down')

        

class FakeCamera():

    
    def __init__(self, topic = 'camera', id = 'camera', file = 'test.jpg'):
        self.image = Image.open(file)
       
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

        
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--topic', type=str, nargs='?', default="camera",
                        help='The registry topic')
    parser.add_argument('--file', type=str, nargs='?', default="test.jpg",
                        help='The file for the fake camera')
    parser.add_argument('--registry', type=str, nargs='?', default=None,
                        help='The IP address of the registry')
    parser.add_argument('--count', type=int, nargs='?', default=10,
                        help='The number of images')
    parser.add_argument('--sleep', type=float, nargs='?', default=0.0,
                        help='The delay between images')
    args = parser.parse_args()
    
    client = RcomWSClient(args.topic, args.topic, args.registry)
    camera = Camera(client)
    #camera = FakeCamera(args.topic, args.topic, args.file)
    for i in range(args.count):
        image = camera.grab()
        if image != None:
            print(f"Saving {args.topic}-{i:05d}.jpg")
            image.save(f"{args.topic}-{i:05d}.jpg")
        time.sleep(args.sleep)
        
