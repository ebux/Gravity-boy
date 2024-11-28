import sys
import os

fname = sys.argv[1]
name = os.path.splitext(fname)[0]
data = open(fname, 'rb').read()

fp = open(f'{name}.h', 'wt')
fp.write(f'unsigned char {name}[]={{\n')
print(len(data))
for i in range(10):
    datastr = ''.join([f'{data[i*16+j]}, ' for j in range(16)])[:-2]
    fp.write(f'    {datastr},\n')    
fp.write('};\n')
fp.close()