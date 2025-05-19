#
# test06.py
#
# a nuPython program of binary expr with semantic error
#
print("")
print("TEST CASE: test06.py")
print("")

x = 3 * 4     # 12
y = abc ** 2  # ERROR
z = 288 / y   # 2
x = 5         # overwrite x to now be 5
remainder = x % z    # 1

print(x)
print(y)
print(z)
print(remainder)

print("")
print("DONE")
print("")
