file = open("connGraph", "w")

n = 2000

for i in range(0,n):
	for j in range(0,n):
		if (i != j):
			file.write("%d\t%d\n" % (i,j))

file.close()

