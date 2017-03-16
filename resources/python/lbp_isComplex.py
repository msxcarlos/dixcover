#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# USAGE
# python recognize.py IMAGE

# RETURNS
# 1 if complex, 0 if simple

# import the necessary packages
import sys
from pyimagesearch.localbinarypatterns import LocalBinaryPatterns
from sklearn.svm import LinearSVC
import cv2
import cPickle
import os


def main(args):
	# initialize image path,
	# local binary patterns descriptor and
	# trained model
	imagePath=args[1]
	if not args[2]:
		modelPath="model.cpickle"
	else:
		modelPath=args[2]
	desc = LocalBinaryPatterns(24, 8)
	model = cPickle.loads(open(modelPath).read())
	
	# load the image, convert it to grayscale, describe it,
	# and classify it
	image = cv2.imread(imagePath)
	gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
	hist = desc.describe(gray)
	hist = hist.reshape(1, -1) #reshape 1d array for use in sklearn model as matrix.
	prediction = model.predict(hist)[0]
	if prediction == "complex_cover":
		isComplex=True
	else:
		isComplex=False
	return isComplex

if __name__ == '__main__':
    sys.exit(main(sys.argv))
