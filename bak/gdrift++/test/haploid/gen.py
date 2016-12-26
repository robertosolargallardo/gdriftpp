import simplejson as json
import sys
from collections import *

data={}

data["individuals"]=[]


id=0
for line in open(sys.argv[1],"r+t"):
   individual={}
   individual["id"]=id

   chromosomes=[]
   chrom={}
   
   chrom["id"]="0"
   genes=[]
   gene={}

   gene["id"]="0"
   gene["sequences"]=[]

   gene["sequences"].append(line)
   genes.append(gene)
   
   chrom["genes"]=genes

   chromosomes.append(chrom)

   individual["chromosomes"]=chromosomes

   data["individuals"].append(individual)

   id+=1

fout=open("out.json","w+t")
json.dump(data,fout)
