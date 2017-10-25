import scipy.ndimage as sp
import scipy.misc as mi
import os
import matplotlib.pyplot as plt
from sklearn.decomposition import PCA
import sys
import re
import shutil
import math


def make_dir(dir):
    if not os.path.exists(dir):
        os.makedirs(dir)


def remove_dir_content(dir):
    for subdir, dirs, files in os.walk(dir):
        for fl in files:
            os.remove(os.path.join(subdir, fl))

train = []
test =[]

print("Processing PCA....");

#rootdir = sys.argv[0]
rootdir = './train'
rootdirt = './test'
size=40

for subdir, dirs, files in os.walk(rootdir):
    if re.split('[/\\\]',string=subdir)[-1] == "train":
        continue
    count = 0
    #print(subdir)
    for fl in files:
        try:
            train.append(mi.imresize(sp.imread(os.path.join(subdir, fl),flatten=True),size=(size,size)))
            count=count+1
        except OSError as e:
            print(e)
    print("Processing PCA train...("+subdir+")"+str(count))

for subdir, dirs, files in os.walk(rootdirt):
    if re.split('[/\\\]',string=subdir)[-1] == "test":
        continue
    count = 0
    #print(subdir)
    for fl in files:
        try:
            test.append(mi.imresize(sp.imread(os.path.join(subdir, fl),flatten=True),size=(size,size)))
            count=count+1
        except OSError as e:
            print(e)
    print("Creating PCA test...("+subdir+")"+str(count))
        
flat = []
flatt = []

for i in range(len(train)):
    flat.append(train[i].flatten())
    
for i in range(len(test)):
    flatt.append(test[i].flatten())

#Start::change below code for heuristic	
#count =  len(train)
#n_comp = math.floor(math.sqrt(0.60 * count))
#n_comp = n_comp * n_comp
#count = min(min(max(n_comp,400),count),400)
#n_comp = math.floor(math.sqrt(count))
#End::change above code for heuristic


count=100
n_comp=10 #it should be the square root of the count


print(count,n_comp)
   
pca = PCA(n_components=count, whiten=True).fit(flat)

eigenfood = pca.components_.reshape((count, size,size))
e_c = min(5,len(eigenfood))
for i in range(0,e_c):
    mi.imsave('./outfile_'+str(i)+'.jpg', eigenfood[i])

train_svm=pca.transform(flat)
test_svm=pca.transform(flatt)


tempdir='./pca_train'
tempdirt='./pca_test'
make_dir(tempdir)
make_dir(tempdirt)
shutil.rmtree(tempdir)
shutil.rmtree(tempdirt)
remove_dir_content(tempdir)
remove_dir_content(tempdirt)
#os.makedirs(tempdir)
#os.makedirs(tempdirt)
i=0
j=0

for subdir, dirs, files in os.walk(rootdir):
    if re.split('[/\\\]',string=subdir)[-1] != "train":
        os.makedirs(os.path.join(tempdir, re.split('[/\\\]',string=subdir)[-1]))
        tempath=os.path.join(tempdir, re.split('[/\\\]',string=subdir)[-1])
        for fl in files:
            mi.imsave(os.path.join(tempath, fl), train_svm[i].reshape((n_comp,n_comp)))
            i = i+1
    
for subdir, dirs, files in os.walk(rootdirt):
    if re.split('[/\\\]',string=subdir)[-1] != "test":
        os.makedirs(os.path.join(tempdirt, re.split('[/\\\]',string=subdir)[-1]))
        tempath=os.path.join(tempdirt, re.split('[/\\\]',string=subdir)[-1])
        for fl in files:
            mi.imsave(os.path.join(tempath, fl), test_svm[j].reshape((n_comp,n_comp)))
            j = j+1
        

#scipy.misc.imsave('C:/Users/adi/Desktop/outfile.jpg', train_svm[0].reshape((10,10)))