battleField = [[0, 1, 1, 0, 0, 1, 0, 0, 1, 0],
[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
[0, 0, 0, 0, 1, 0, 1, 0, 0, 1],
[0, 0, 0, 0, 1, 0, 0, 0, 0, 0],
[0, 0, 0, 0, 1, 0, 0, 0, 0, 0],
[1, 1, 1, 0, 0, 0, 0, 0, 0, 0],
[0, 0, 0, 0, 0, 0, 1, 0, 0, 0],
[0, 0, 0, 0, 0, 0, 1, 0, 0, 0],
[0, 0, 0, 0, 1, 0, 1, 0, 0, 0],
[0, 1, 1, 0, 1, 0, 1, 0, 0, 0]]

def check_ship_vert(field, i, j, size):

	if j > 0 and ((i > 0 and field[i-1][j - 1]) or field[i][j - 1] or ((i < 9 and field[i+1][j - 1]))):
		return -1
	elif ((i > 0 and field[i-1][j])):
		return -1
	elif j < 9 and ((i > 0 and field[i-1][j + 1]) or ((i < 9 and field[i+1][j + 1]))):
		return -1

	if i < 9 and field[i + 1][j] == 1:
		size = size + 1
		if size > 4:
			return -1
		return check_ship_horiz(field, i + 1, j, size)
	else:
		return size

def check_ship_horiz(field, i, j, size):
	
	if i > 0 and ((j > 0 and field[i - 1][j - 1]) or field[i - 1][j] or ((j < 9 and field[i - 1][j + 1]))):
		return -1
	elif (size == 1 and (j > 0 and field[i][j - 1])):
		return -1
	elif i < 9 and ((j > 0 and field[i + 1][j - 1]) or field[i + 1][j] or ((j < 9 and field[i + 1][j + 1]))):
		print(field[i + 1][j - 1], field[i + 1][j], field[i + 1][j + 1])
		return -1

	if j < 9 and field[i][j + 1] == 1:
		size = size + 1
		if size > 4:
			return -1
		return check_ship_vert(field, i, j + 1, size)
	else:
		return size

def validate_battlefield(field):
		# your code here
		ships = {
			4: 1,
			3: 2,
			2: 3,
			1: 4
		}

		for i in range(10):
			for j in range(10):
				if field[i][j] == 1:
					size = check_ship_horiz(field, i, j, 1)
					if size > 1:
						ships[size] = ships[size] - 1
						if ships[size] < 0:
							return False
						j += size - 1
						continue
					else:
						size = check_ship_vert(field, i, j, 1)
						if size >= 1:
							ships[size] = ships[size] - 1
							if ships[size] < 0:
								return False

		for key in ships:
			if ships[key] != 0:
				return False

		return True

print(battleField[0][1])

print(validate_battlefield(battleField))