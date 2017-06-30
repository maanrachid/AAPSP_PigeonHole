
#include <iostream>

#include <omp.h>
#include "stdlib.h"
#include "Tools.h"
#include "testutils.hpp"
#include "Ctree.h"
#include <stdio.h>
#include <string.h>
#include <list>
#include <unordered_map>
#include <string>
#include <sstream>
#include <math.h>

#define MAX_K 1000000
#define MAX_N 1400000000
#define BUFFER 10



using namespace std;

void displayhelp();
void print_Sorted(uchar *a,uint *So);
void print(uchar *a);
bool isTerminator(uchar i);
bool isTerminatorfile(uchar i);
int choose_index(int a);
void display_choices(char *filename, char *output, int threads,int min,int mismatches);
void encode(uchar *final,ulong counter,int bitnum,char c);
bool compare(int start,string str,int r1,int r2,int sizeToCompare,uchar *text,int *mismatches);
ulong N;
uint K;
ulong *startS;




void APSP(char *filename, char *output, int threads,int min,int mismatches)
{
    


  // N is the total length of the all strings. we have k strings.   
  //uint *Sorted= new uint[MAX_K];
  uchar *text;
  std::ifstream::pos_type posSize;
  std::ifstream file ((char *)filename, std::ios::in|std::ios::binary|std::ios::ate);
  ulong counter=0,bitnum=1,reminder=0,counterk=0,pos=0;
  // counterk is to count strings, counter to count the size of text 
  //(without the seperators), pos to get the current pos in final array.
  startS = new ulong[MAX_K];
  struct stack_node **matched = new stack_node*[MAX_K];    /* used for matched strings in the modified tree */
  
  startS[0]=0;

  unordered_map<string, struct dicLink *> * index =  new unordered_map<string, struct dicLink *>[mismatches+1];
  int piecesize = min/(mismatches+1);
  cerr<<"Sigment Size:"<<piecesize<<endl;
  


  if (file.is_open())
    {
      posSize = file.tellg();
      ulong size = posSize;
      if (MAX_N != 0 && size > MAX_N)
	size = MAX_N;
      char *memblock = new char [size/BUFFER + 1];
      text = new uchar[size/4];
      reminder = size%BUFFER;

      string str1="";
      int m=0;

      for(int i=0;i<BUFFER;i++){
	file.seekg (i*(size/BUFFER), std::ios::beg);
	file.read (memblock, size/BUFFER);
	for(ulong z=0;z<size/BUFFER;z++){
	  if (memblock[z]!=SEPERATOR){
	    //cerr <<"encoding:"<<memblock[z]<<" pos:"<<pos<<endl;
	    encode(text,pos,bitnum,memblock[z]);
	    if (memblock[z]=='A' || memblock[z]=='C' || memblock[z]=='G' || memblock[z]=='T') {
	      counter++;
	      bitnum+=2;
	      //cerr<<"bitnum now "<<bitnum<<endl;
	      if (bitnum==9) {
		bitnum=1;
		pos++;
	      }
	      
	      if (m<min){
		m++;
		str1+= memblock[z];
		if (m==min) {
		  for(int ii=0; ii<(mismatches+1);ii++){
		    string part = str1.substr(ii*piecesize,piecesize);
		    //cout << part <<endl;
		    struct dicLink *dl = new dicLink();
		    dl->read = counterk;
		   
		    if (index[ii].find(part)!=index[ii].end()){
		      struct dicLink *temp = index[ii][part];
		      index[ii][part]= dl;
		      dl->link =temp;
		    }else {
		      dl->link= NULL;
		      index[ii][part]= dl;
		    }
		    
		  }
		  
		}
	      }
	    }
	  }else {
	    startS[++counterk]=counter;
	    //Sorted[counterk-1]=counterk-1;
	    matched[counterk-1]=NULL;
	    m=0;
	    str1="";
	  }
	}
      }

      //cerr <<"reminder:"<<reminder<<endl;
      if (reminder>0){
	file.seekg (BUFFER*(size/BUFFER), std::ios::beg);
	file.read (memblock, reminder);
	for(ulong z=0;z<reminder;z++){
	  if (memblock[z]!=SEPERATOR){
	    encode(text,pos,bitnum,memblock[z]);
	    if (memblock[z]=='A' || memblock[z]=='C' || memblock[z]=='G' || memblock[z]=='T') {
	      counter++;
	      bitnum+=2;
	      if (bitnum==9) {
		bitnum=1;
		pos++;
	      }
	    }
	  }else {
	    startS[++counterk]=counter;
	    //Sorted[counterk-1]=counterk-1;
	    matched[counterk-1]=NULL;
	  }
	}
      }

      startS[counterk]=counter;
      //Sorted[counterk]=counterk;
      matched[counterk]=NULL;
		
      N = counter;
      file.close();
      cerr<<"Size of File:" << size << endl;
      cerr<<"Number of Strings: K "<< counterk<<endl;
      cerr <<"Size of strings:" << counter <<endl;
      K=counterk;

      
      /* decoding code
	 for(int i=0;i<counterk;i++){
	 //cerr << "start" <<i<< ":"<<startS[i]<<endl;
	 for(ulong z=startS[i];z<startS[i+1];z++){				
	 char v= decode(text,z);
	 //cerr<<"decoding byte "<<bbyte<<" bit "<<bbit<<" "<<v <<endl;
	 //cerr<<v;
	 }
	 //cerr <<SEPERATOR;
	 }*/

      /* check the contents of the indices 
      for(int ii=0;ii<mismatches+1;ii++){
	for(unordered_map<string, struct dicLink*>::iterator zz=index[ii].begin();zz!=index[ii].end();zz++){
	  cout<<ii<<" "<<zz->first<<" "<<zz->second->read ;
	  struct dicLink *ff = zz->second->link;
	  while (ff!=NULL){
	    cout << " " << ff->read;
	    ff= ff->link;
	  }
	  cout<<endl;
	}
	}  */
    }

  omp_set_num_threads(threads); 
 
	
  double starttime,endtime;
  starttime = omp_get_wtime();
  cerr<<"Finding Approximate Overlaps: "<<endl;
 
 
 

#pragma omp parallel for 
  for(ulong i=0;i<counterk;i++){
    ulong b=startS[i];
    ulong next=startS[i+1];
    string s="";
    for(int j=b;j<next;j++){
      s+= decode(text,j); 
    }

    

    
    //cerr << i<< " "<< (int)(s.length())-min+1<<endl;
    int len=s.length();


    for(int l=0;l<len-min+1;l++){ // check all suffixes in string i
     
      string str1 = s.substr(l,len-l);
      unordered_map<int,int> *printed = new unordered_map<int,int>();
      
      //cout<<"string :"<<str1<<endl;
      for (int z=0;z<=mismatches;z++){
	string parts = str1.substr(z*piecesize,piecesize);
	//cout<<parts<<endl;
	unordered_map<string, struct dicLink *>::iterator itr= index[z].find(parts);
	if (itr!=index[z].end()){
	  //cout << "found match " <<parts<<"from "<<str1<<" in index " <<z <<endl;
	  struct dicLink *ptr=itr->second;
	  list<int> matchlist;
	  while (ptr!=NULL){
	    int v=mismatches;
	    int r=ptr->read;
	    bool result=false;
	    


	    if (r==i)
	      { ptr=ptr->link;continue;}
	    if (z==0)
	      result=compare(piecesize,str1,i,r,str1.length()-piecesize,text,&v);
	    else if (z==mismatches){
	      result= compare(0,str1,i,r,z*piecesize,text,&v);
	      if (v>=0) 
		result= compare(piecesize*(z+1),str1,i,r,str1.length()-piecesize*(z+1),text,&v);
	      else
		result=false;
	    }else {
	      result = compare(0,str1,i,r,z*piecesize,text,&v);
	      if (v>=0) 
		result= compare((z+1)*piecesize,str1,i,r,str1.length()-piecesize*(z+1),text,&v);
	      else
		result=false;
	    }

	    if (result && i!=r){
	      //stringstream ss;
	      //string h;
	      //ss << i <<" "<<r <<"->"<<str1.length()<<endl;
	      if (output[0]!='0' && printed->find(r)==printed->end()){
		
		printf("%d %d-> %d\n",i,r,str1.length());
		(*printed)[r]=1;
		
	      }
	      //ss>> h;
	      //matchinglist.push_back(h);
	      //return;
	    }
	    
	    ptr=ptr->link;
	  }
	}
      }

      printed->clear();
      delete printed;
      
    }
  }
 endtime = omp_get_wtime();  

  cerr<<"User Time for finding approximate overlaps: "
      <<endtime-starttime<<endl;

    


 
  //cerr<<"displaying tree: "<<endl;
  //display_tree(ptr);
  //print(text);
  //print_Sorted(text,Sorted);
  
  

  //  if (method[0]=='0'){
  //find_all_pairs(ptr,text,N,K,startS,Sorted,atoi(output),threads,distribution_method,min,index);
  // }else {
  //find_all_pairs_modified(ptr,text,N,K,startS,Sorted,atoi(output));
  //}


  //cerr<<"Press any key to continue: ";
  //cin >>N;

  // Clean up    
  delete [] startS;
    
}

bool compare(int start,string str,int r1,int r2,int sizeToCompare,uchar *text,int *mismatches){
  int i;

  //cout<<"comparing "<<str<< " in "<<r1 << " and "<<r2<<" from " << start << " "<<sizeToCompare<<endl; 
  for(i=start;i<start+sizeToCompare;i++){
    //cout << str[i]<<" "<<decode(text,startS[r2]+i)<< " " << *mismatches<<endl;
   
    
    if (startS[r2]+i>=startS[r2+1]){ // added for cases when a suffix is bigger than a string
      return false;
    }
    
    if ((str[i]!=decode(text,startS[r2]+i)) /*|| (startS[r2]+i>=startS[r2+1])*/){ 
      (*mismatches)--;
      if ((*mismatches)==-1) return false;
    } 
  }

  return true;
}




int main(int argc,char *argv[])
{

  char *filename;

  char *output=new char[2];
  int mismatches,processors,minlength;
  // Default values 
  
  processors = omp_get_num_procs();
  minlength = 1;
  output[0] = '1';

  
  if (argc<2){
    cerr <<"wrong number of arguments, run Apsp to get a help"<<endl;
    displayhelp();
    return 0;
  }
  
  filename = argv[1];
  mismatches = 3;
  

  for(int i=2;i<argc;i++){
    if (argv[i][0]=='-'){
      if (argv[i][1]=='p')
	processors = atoi(argv[i+1]);
      else if (argv[i][1]=='m')
	minlength=atoi(argv[i+1]);
      else if (argv[i][1]=='h')
	mismatches=atoi(argv[i+1]);
      else if (argv[i][1]=='o')
	output=argv[i+1];
    }
  }

  display_choices(filename,output,processors,minlength,mismatches);

  /*
    if (argc!=6){
    cerr <<"wrong number of arguments"<<endl;
    cerr <<"Apsp filename output sorting threads distribution_method"<<endl;
    return 0;
    }*/
  
  APSP(filename,output,processors,minlength,mismatches);
  return 0;
}





int choose_index(int a){
  if (a=='A') 
    return 1;
  else if (a=='C')
    return 2;
  else if (a=='G')
    return 3;
  else if (a=='T')
    return 4;
  else
    return 0;
}




bool isTerminatorfile(uchar i){

  return (i==SEPERATOR);
}

void print(uchar *a){
  int con=0;
  int strs=0;
  while (a[con]!='\0'){
    if (a[con]=='#'){ 
      cout<<" "<<strs<<endl;strs++; 
    }else 
      putchar(a[con]);
    con++;
  }

  cout<<"------------" <<endl;
}

void print_Sorted(uchar *a,uint * So){
  int con=0;

  for(uint i=0;i<K;i++){
    con=0;
    while (startS[So[i]] + con  /*a[startS[So[i]]]*/< startS[So[i]+1])    /*a[startS[So[i]+1]]*/{
      putchar(decode(a,startS[So[i]]+con)   /*a[startS[So[i]]+con]*/);
      con++;
    } 
    cout << " "  <<endl;
  }

  cout<<"------------" <<endl;
}



void displayhelp(){
  cerr<<"*******************************************************"<<endl;
  cerr<<"This program has one parameter and four optional parameters:"<<endl;
  cerr<<"Apsp_first filename [mismatches] [number_of_processors] [output] [minimum_length]"<<endl;
  cerr<<"filename\t is the name of the data file."<<endl;
  cerr<<"-h\tnumber of mismatches. Default value is 3."<<endl;
  cerr<<"-p\tnumber of processors(threads). Default value is the maximum."<<endl;
  cerr<<"-o\toutput:\n\t 1 (default) for producing an output, 0 for no output."<<endl; 
//in an array.\n\t 0 for executing without an output.\n\t 2 for outputing all overlaps."<<endl;
  cerr<<"-m\tminimum length for matching, 1 is the default value."<<endl;
  cerr<<"*******************************************************"<<endl;
}


void display_choices(char *filename, char *output, int threads,int min,int mismatches){

  cerr<<"These are your choices:"<<endl;
  cerr<<"File name:"<<filename<<endl;
  cerr<<"Output:";
  if (output[0]=='0')
    cerr<<" No Output" <<endl;
  // else if (output[0]=='1')
  // cerr<<" Maximum suffix-prefix matches are shown in two dimentional array."<<endl;
  else
    cerr<<" All overlaps are shown "<<endl;


  cerr<<"Number of threads:"<<threads<<endl;
  cerr<<"Minimum Match Length:"<<min<<endl;
  cerr<<"Number of Mistmatches:"<<mismatches<<endl;;
}

void encode(uchar *final,ulong counter,int bitnum, char c){
  if (c=='A'){
    final[counter] &= ~(1<<bitnum);
    final[counter] &= ~(1<<((bitnum+1)%8));
  }else if (c=='C'){
    final[counter] |= (1<<bitnum);
    final[counter] &= ~(1<<((bitnum+1)%8));
  }else if (c=='G'){
    final[counter] &= ~(1<<bitnum);
    final[counter] |= (1<<((bitnum+1)%8));
  }else if (c=='T'){
    final[counter] |= (1<<bitnum);
    final[counter] |= (1<<((bitnum+1)%8));
  }
}


