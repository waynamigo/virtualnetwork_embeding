#ifndef BASE_H_INCLUDED
#define BASE_H_INCLUDED
#include<iostream>
#include<cmath>
#include<fstream>
#include<string>
#include<vector>
#include<map>
#include<set>
#include<stack>
#include<algorithm>
#include<list>
#include<queue>
#include<cstring>
#include<string>
using namespace std;
#define MAX_REQ_PER_NODE 1000
#define MAX_REQ_LINKS 100
#define infinity 999999
struct Link{
	int linkId;
	int from;
	int to;
	double bandwidth;
	double residual_bw;
	double ratio;
	double weight;
	int hostVirtualLinkCount;//已经映射了虚拟链路的条数
	int virtualLink[MAX_REQ_LINKS];//虚拟请求链路数组
	double BW[MAX_REQ_LINKS];//带宽数组
};
struct Node{
	int nodeId;
	double CPU;
	double residual_cpu;
	double ratio;
	int hops;
	double H;
	set<int> linkNeighbors;//邻接链路
	set<int> nodeNeighbors;//邻接节点id
    int x,y;
};
struct CopySub{
	int nodes;
	int links;
	vector<vector<int> > path;
	vector<vector<double> > weight;
	void print_copy(){
		cout<<nodes<<','<<links<<endl;
		for (int k = 0; k < nodes; ++k) {
			cout<<k<<' ';
		}
		cout<<endl;
		for(int i=0;i<nodes;i++) {
			for(int j =0;j<nodes;j++){
				cout<<path[i][j]<<' ';
			}
			cout<<'\n';
		}
	}
};
#endif // BASE_H_INCLUDED
