def eith(i):
  return i + (8-i%8)
if __name__ == "__main__":
  for i in range(100):
    print "closest eith to",i,"is",eith(i)
