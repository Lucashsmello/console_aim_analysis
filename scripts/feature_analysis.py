from sys import argv
from math import sqrt
import csv
from sklearn import linear_model
from sklearn.metrics import mean_squared_error
import random

def extractRandom(x,y):
	return random.random() * 60 

def extractCosineX(x,y):
	return x*x/extractRadius(x,y)

def extractYXrate(x,y):
	return float(y)/x

def extractGeometricMean(x,y):
	return sqrt(x*y)
	
def extractRadius(x,y):
	return sqrt(x*x+y*y)
	
def featureExtraction(data_fname, out_fname, extractors_list):
	DP="%.5f"
	newDataX=[]
	dataY=[]
	with open(data_fname, 'rb') as input_data, open(out_fname,'wb') as out_data:
		csvreader = csv.reader(input_data,delimiter=';')
		csvwriter = csv.writer(out_data,delimiter=';')
		for row in csvreader:
			x=int(row[0])
			y=int(row[1])
			newfeats=[x,y]
			for extr in extractors_list:
				newfeats.append(DP % extr(x,y))
			newfeats.append(row[2])
			csvwriter.writerow(newfeats)
			newDataX.append([float(f) for f in newfeats[:-1]])
			dataY.append(float(newfeats[-1]))
	return [newDataX,dataY]
			

def holdOut(data_in,data_out,feature_list, hold_percent=0.66):
	[X,Y]=featureExtraction(data_in,data_out,feature_list)
	n=len(X)
	m=len(feature_list)
	train_size=int(hold_percent*n)
	R=range(n)
	random.shuffle(R)
	X_train=[X[i] for i in R[:train_size]]
	X_test=[X[i] for i in R[train_size:]]
	Y_train=[Y[i] for i in R[:train_size]]
	Y_test=[Y[i] for i in R[train_size:]]
	reg = linear_model.LinearRegression()
	reg.fit(X_train,Y_train)
	y_pred = reg.predict(X_test)
	y_pred_train = reg.predict(X_train)
	print(reg.coef_,reg.intercept_)
	print("TTRAIN Mean squared error: %.2f" % (mean_squared_error(Y_train, y_pred_train)))
	print("TEST Mean squared error: %.2f" % (mean_squared_error(Y_test, y_pred)))
			
if(len(argv)<=2):
	print("ARGS: INPUT_DATA OUT_DATA")
else:
	holdOut(argv[1],argv[2],[extractRadius, extractYXrate, extractCosineX,extractRandom],0.5)
	
	
	
	
	
	
	