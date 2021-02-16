import random
import sys

res = [random.randint(-100,100) for i in range(int(sys.argv[1]))]

print(' '.join(map(str,res)))
