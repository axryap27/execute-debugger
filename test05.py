#
# test05.py
#
# a nuPython program of binary expr with semantic error
#
print("")
print("TEST CASE: test05.py")
print("")

x = 3 * 4     # 12
y = x ** 2    # 144
z = 288 / fred   # ERROR
x = 5         # overwrite x to now be 5
remainder = x % z    # 1

print(x)
print(y)
print(z)
print(remainder)

print("")
print("DONE")
print("")
