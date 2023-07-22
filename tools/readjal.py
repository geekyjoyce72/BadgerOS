#!/usr/bin/env python3

import sys



def read_jal(raw):
	u32  = ((raw >> 20) & 0x0000_07fe)
	u32 |= ((raw >> 9)  & 0x0000_0800)
	u32 |= ((raw)       & 0x000f_f000)
	u32 |= ((raw >> 11) & 0x0010_0000)
	s32  = (u32 | 0xffe0_0000) if (raw & 0x8000_0000) else u32
	return s32



if __name__ == "__main__":
	if len(sys.argv) < 2:
		print("Usage: readjal.py [instruction...]")
	for i in range(1, len(sys.argv)):
		n=read_jal(int(sys.argv[i], base=16))
		print(hex(n))
