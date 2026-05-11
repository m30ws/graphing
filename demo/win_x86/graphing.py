import os
import random
import socket
import subprocess as subp
import sys
import time


if __name__ == '__main__':
	GRAPHGRID_PORT = 5056
	BINFILE = "./graphing.exe"

	if len(sys.argv) > 1:
		NGRAPHS = int(sys.argv[1])
	else:
		NGRAPHS = 10

	sock = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
	sock.bind(("localhost", GRAPHGRID_PORT))
	print("started server.")

	with open("dump.txt", "w") as fp:
		os.environ["PYTHONUNBUFFERED"] = "1"
		graph_sub = subp.Popen([BINFILE, str(NGRAPHS)], text=True, universal_newlines=True, stdin=subp.PIPE, stdout=sys.stdout, stderr=fp)

		def rand_n_ints(a, n):
			return list(map(str, random.sample(range(-a, a), n ) ))
		def rand_n_floats(a, n):
			return list(map(str, [random.uniform(-a, a) for _ in range(n)]))

		special = [
			# Sprinkle some special sparkle
			[float(i) for i in range(0, 1000+1, (1000+1-0)//500)],
			[float(1000) for i in range(500//2)] + [float(0) for i in range(500//2)],
			[float(i) for i in range(1000, 0-1, (0-1-1000)//500)],
			[float(-1000) for i in range(500//2)] + [float(1000) for i in range(500//2)],
		]
		idx = 0

		while True:
			# print(f"> ", end='')
			# inp = input(); print()

			if graph_sub.poll() is not None:
				break # .poll()==None => subprocess is dead

			# Simulate random values
			# ======================
			amp = random.randint(100, 1000)

			ss = ', '.join(
				[str(ii[idx % len(ii)]) for ii in special]
				# + rand_n_ints(amp, NGRAPHS - len(special))
				+ rand_n_floats(amp, NGRAPHS - len(special))
			)

			# Or receive from the socket
			# ==========================
			# ss = sock.recvfrom(65535)
			# ss = ss[0].decode('utf-8')

			# print(f"ss: {ss}")
			# if (ss == ">stop"):
			# 	break

			ss = ss.replace(' ', ", ")

			graph_sub.stdin.write(f"{ss}\n")
			graph_sub.stdin.flush()			

			idx += 1
			time.sleep(0.001) # Sleep for some time

		graph_sub.wait(3)