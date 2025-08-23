import time
from rcom.rcom_client import RcomWSClient
import argparse


class BatteryMonitor():

    @staticmethod
    def create(topic, registry_ip = None):
        client = RcomWSClient(topic, topic, registry_ip)
        return BatteryMonitor(client)
   
    def __init__(self, client):
        self.client = client
        
    def is_charging(self):
        return self.client.execute('battery-monitor:is-charging')
        
    def get_voltage(self):
        return self.client.execute('battery-monitor:get-voltage')
        
    def get_current(self):
        return self.client.execute('battery-monitor:get-current')

        
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--topic', type=str, nargs='?', default="battery",
                        help='The registry topic')
    parser.add_argument('--registry', type=str, nargs='?', default=None,
                        help='The IP address of the registry')
    args = parser.parse_args()
    
    monitor = BatteryMonitor.create(args.topic, args.registry)
    while True:
        print(f"Charging: {monitor.is_charging()}")
        print(f"Voltage:  {monitor.get_voltage()} V")
        print(f"Current:  {monitor.get_current() A}")
        time.sleep(1)
