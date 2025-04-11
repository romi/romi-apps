import argparse
import time
from romi.cnc import CNC


parser = argparse.ArgumentParser()
parser.add_argument('--registry', type=str, nargs='?', default="127.0.0.1",
                        help='IP address of the registry')
args = parser.parse_args()
    
cnc = CNC("cnc", "cnc")
cnc.power_up()
cnc.homing()

dimension = cnc.get_range()
print(f"dimension={dimension}")

xc = (dimension[0][0] + dimension[0][1]) / 2.0
yc = (dimension[1][0] + dimension[1][1]) / 2.0
d = min(dimension[0][1] - dimension[0][0],
        dimension[1][1] - dimension[1][0])
r = (d - 0.02) / 2
print(f"c=({xc}, {yc}), r={r}")
    
cnc.moveto(xc - r, yc, 0.0, 0.5)    
cnc.helix(xc, yc, -2.0 * math.pi, -2.0 * math.pi, 1.0)

position = cnc.get_position()
print(f"position={position}")
    
n = 32
delta_alpha = -2.0 * math.pi / n
start_time = time.time()
for i in range(n):
    cnc.helix(xc, yc, delta_alpha, delta_alpha, 1.0)
    print(f"duration: {time.time()-start_time}")
