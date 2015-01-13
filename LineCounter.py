#*********************
#*Author:YuliWANG@SunYatSenUniv.
#*Lang:Python
#*Date:
#*********************
import os
import re
py = re.compile(r'.\.py$')
h=re.compile(r'.\.h$')
cpp = re.compile(r'.\.cpp$')
def getLines(path):
	# print path
	sum=0
	for root,dirs,fns in os.walk(path):
		if root.find('vlc')!=-1:
			continue
		absolutePath=root+"\\"
		for f in fns:
			if cpp.search(f) or py.search(f) or h.search(f):
				print root,dirs,f
				fin=open(absolutePath+f,"r")
				sum+=len(fin.readlines())
		# for dir in dirs:
		# 	temp=getLines(path+"\\"+dir)
		# 	sum+=temp
		# 	if temp:
		# 		print path+"\\"+dir,": ",temp		
	return sum

if __name__=="__main__":
	print "Counter of Code Lines Running...\nYuliWANG@SunYatSenUniv."
	print getLines(os.getcwd())
