#include <iostream>
#include"Base.h"
#include"Request.h"
#include"Substrate.h"
#include<string>
using namespace std;
bool node_mapping(SubstrateNetwork &sn, Request &request, map<int,int> &vnMap){
	//遍历物理节点是否已经映射的结果
	map<int,bool> snMapped;
	for(int i=0;i<sn.nodes;i++) snMapped[i]=false;
	set<int> setMappedSN;//已经映射的SN节点集合
	setMappedSN.clear();//用于计算未来映射的节点和已经映射节点集合距离时使用。
	for(int i=0;i<request.mapNodes.size();i++){//虚拟网络也用H值最大的节点开始映射
		double need_cpu=request.mapNodes[i].CPU;//虚拟节点所需求的CPU容量大小
		vector<int> vecSN;//存放物理网络节点的向量
		for(int k =0 ;k<sn.nodes;k++){
			vecSN.push_back(sn.mapNodes[k].nodeId);//将物理节点按H值排序后的序号记录下来
		}
		//根据H值降序排列
		for(int j=0;j<vecSN.size();j++){
			if(!snMapped[vecSN[j]]){
				if(sn.mapNodes[vecSN[j]].residual_cpu > request.mapNodes[i].CPU){
					vnMap[i] = vecSN[j];//节点映射结果存放
					snMapped[vecSN[j]] = true;//该物理节点映射完毕
					// sn.mapNodes[vecSN[j]].CPU-=request.mapNodes[request.mapNodes[i].nodeId].CPU;
					setMappedSN.insert(vecSN[j]);
					break;
				}
			}
			if(j==sn.nodes){//映射失败
				return false;
			}
		}
	}
	return true;
}
bool Distri_nodes(SubstrateNetwork &substrateNetwork, Request &request, map<int,int> &resultnodes){
	for(auto it = resultnodes.begin(); it!=resultnodes.end();it++){
		substrateNetwork.mapNodes[it->second].residual_cpu=substrateNetwork.mapNodes[it->second].residual_cpu-request.mapNodes[it->first].CPU;//剩余CPU变化
		substrateNetwork.mapNodes[it->second].ratio=1.0-substrateNetwork.mapNodes[it->second].residual_cpu/substrateNetwork.mapNodes[it->second].CPU;//计算利用率
        cout<<it->second<<':'<<substrateNetwork.mapNodes[it->second].CPU<<"   "<<substrateNetwork.mapNodes[it->second].residual_cpu<<endl;
    }
	return true;
}
//打印节点映射结果
void print_noderesult(map<int,int> &resultnodes){
	for(auto it=resultnodes.begin(); it!=resultnodes.end();it++){
		cout<<"vn:"<<it->first<<"->sn:"<<it->second<<endl;
	}
}
bool link_mapping(SubstrateNetwork &substrateNetwork,Request &request,CopySub &copySub,map<int,int> &resultnodes,map<int, vector<int>> &resultlinks){
	 for(auto iterLinks =request.mapLinks.begin(); iterLinks!=request.mapLinks.end();iterLinks++){
        int linkId=iterLinks->first;//链路的id
        Link vlink=iterLinks->second;//request的一条链路需求
        double need_bw = vlink.bandwidth;
        int from = resultnodes[vlink.from];
        int to = resultnodes[vlink.to];
        vector<int> resultSubpath;//最短路向量 内容如1->8->9 经过物理节点1 8 9
		resultSubpath.clear();
        bool iffinded = substrateNetwork.findShortestByVirtualFrom_To (copySub,from,to,resultSubpath);
        substrateNetwork.displayShortest(copySub,from,to);
        if(iffinded){
	 		//验证带宽
	 		for(int k=0;k<(resultSubpath.size()-1);k++) {
				int iStart = resultSubpath[k];
				int iEnd = resultSubpath[k + 1];
				int sublinkId = substrateNetwork.getLinkId(iStart,iEnd);
				if(need_bw > substrateNetwork.mapLinks[sublinkId].residual_bw) {
					return false;
				}
	 		}
	 		resultlinks[linkId] = resultSubpath;//将这一条链路加入resultLinks，链路映射最终结果
	 	}else{
	 		return false;//链路不通，false
	 	}
	 }
	return true;
}
bool Distri_links(SubstrateNetwork &substrateNetwork,Request &curReq,map<int, vector<int>> &resultlinks){
    for(int i=0; i<curReq.links; i++){
        double need_bw = curReq.mapLinks[i].bandwidth;//当前虚拟请求需要的带宽
        for(int j=0; j<resultlinks[i].size()-1; j++){
            int from = resultlinks[i][j];//这里取出的是每条物理链路的两个物理顶点 a和b
            int to = resultlinks[i][j+1];
            //根据开始和结束，找到对应的物理链路的索引号
            int eachSubstrateLinkIndex = substrateNetwork.getLinkId(from,to);
            if(eachSubstrateLinkIndex == -1||substrateNetwork.mapLinks[eachSubstrateLinkIndex].residual_bw < need_bw){
                cout<<"映射失败"<<endl;
                return false;
            }
            substrateNetwork.mapLinks[eachSubstrateLinkIndex].residual_bw -= need_bw;//带宽有所消耗
            substrateNetwork.mapLinks[eachSubstrateLinkIndex].ratio=1.0-substrateNetwork.mapLinks[eachSubstrateLinkIndex].residual_bw/substrateNetwork.mapLinks[eachSubstrateLinkIndex].bandwidth;//utilization ratio of BW
            substrateNetwork.mapLinks[eachSubstrateLinkIndex].hostVirtualLinkCount++;//物理链路所服务的虚拟链路条数加1
        }
    }
    return true;
}

//	cout<<(double)1.0/get_Cent(substrateNetwork,0);
		// cout<<"以下是更新完带宽后物理网络的内容";
		// substrateNetwork.print();

//         cout<<"物理节点顺序\n";substrateNetwork.print();
//         cout<<"虚拟节点顺序\n";request.print();cout<<endl;
//         map<int,int> resNodes;
    
//         map<int,int> resLinks;
//         testSimulator();
//         BFS_VNM(substrateNetwork,request,resNodes,resLinks);
//     //    cout<<"余下的未映射链路方案显示于floyd算法计算的路径\n";
//     //    for(map<int,int>::iterator iter=resNodes.begin();iter!=resNodes.end();iter++){
//     //        cout<<iter->first<<'\t'<<iter->second<<endl;
//     //    }
// //        cout<<"按照requests的编号,对应的物理链路映射的顺序"<<endl;
// //        for(map<int,int>::iterator iter=resLinks.begin();iter!=resLinks.end();iter++){
// //            cout<<iter->first<<'\t'<<iter->second<<endl;
// //        }
//         double r_c = request.getR_C(shortestpath,setMapped);
//         cout<<"收益开销比：" <<r_c<<endl;
//         char resultfile[]="/home/waynamigo/Desktop/myDocuments/c++/rc&acc&sf";
//         fprintSF(resultfile,resLinks,request);
        // ofstream fout(resultfile,ios::app);	
