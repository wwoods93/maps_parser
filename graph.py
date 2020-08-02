# graph.py
# Creates a bar graph depicting mapping size, RSS, and dirty bytes
import pandas as pd
import matplotlib.pyplot as plt

ramArray = []
swapArray = []
dirtyArray = []

inFile = open("appTotals0.txt")
data = inFile.read().splitlines()

for i in range(0, len(data)):
    temp = data[i].split()
    ramArray.append(int(temp[1]))
    swapArray.append((int(temp[0])-(int(temp[1])+int(temp[2]))))
    dirtyArray.append(int(temp[2]))

df = pd.DataFrame({
    'RAM': ramArray,
    'SWAP': swapArray,
    'Dirty': dirtyArray
})

plott = df.plot(kind='bar', stacked=True)
plott.set_xlabel("Memory Locations")
plott.set_ylabel("Size in Kb")
plt.show()
