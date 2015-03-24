import os
import random

STR_PATH_PREF = "files"
FILES_COUNT = 10
MAX_RAND_INT = 5
MIN_RAND_INT = 0

if not os.path.exists(STR_PATH_PREF):
	os.makedirs(STR_PATH_PREF)

for i in range(FILES_COUNT):
	fname  = "{0}/file{1}.txt".format(STR_PATH_PREF, str(i))
	number = random.randint(MIN_RAND_INT, MAX_RAND_INT)
	open(fname, "w").write(str(number))