fp = open('tables.h', 'wt')

fp.write(f'const unsigned char table_mod3[] = {{')
for i in range(96//16):
    table_data = ''.join([f'{j%3}, ' for j in range(i*16,(i+1)*16)])
    fp.write(f'{table_data}\n')
fp.write('};\n')

fp.write(f'const unsigned char table_div3[] = {{')
for i in range(96//16):
    table_data = ''.join([f'{j//3}, ' for j in range(i*16,(i+1)*16)])
    fp.write(f'{table_data}\n')
fp.write('};\n')

fp.write(f'const unsigned int table_ypos[] = {{')
table_data = ''.join([f'{hex(0xf800+64*(j//3))}, ' for j in range(96)])[:-2]
fp.write(f'{table_data}}};\n')

fp.write(f'const unsigned char *tile_pos[] = {{')
for i in range(40):
    fp.write(f'tiles+{i*12},\n')
fp.write(f'}};\n')

fp.close()