#include <iostream>
#include"Base.h"
#include"Request.h"
#include"Substrate.h"
#include<string>
using namespace std;
void test(Request request){
    int from,to;
    from =1;
    to =2;
    cout<<request.iflinked(from,to);
}
double getCost(Request &request,map<int,int> &resultNodes,map<int,int> &resultLinks){//获得映射成功request网络的开销
    double cost=0;
	double cpu_revenue = 0;
	double bw_revenue = 0;
	for(int i=0;i<request.nodes;i++){
		cpu_revenue += request.mapNodes[i].CPU;//计算一组（一个request）请求的总CPU
	}
	for(int i=0; i<request.links; i++){
		bw_revenue +=(request.mapLinks[i].bandwidth*(resultLinks.size()-1));//计算bandwidth
	}
	cost=(cpu_revenue + bw_revenue)*(request.duration);
	return cost;
}
double getRev(Request &request,map<int,int> &resultNodes,map<int,int> &resultLinks){//获得request的实际收益
    //vector<int>存取比较方便，但是改回来太麻烦 不改了
	double revenue=0;
	double cpu_revenue = 0;
	double bw_revenue = 0;
	for(int i=0;i<request.nodes;i++){
		cpu_revenue += request.mapNodes[i].CPU;
	}
	for(int i=0; i<request.links; i++){
		bw_revenue +=(request.mapLinks[i].bandwidth);
		if(resultLinks.size()==0){
            break;//映射失败 直接跳出，此时revenue=
        }
    }
	revenue = (cpu_revenue + bw_revenue)*(request.duration);
	return revenue;
}

double getSF(Request &request,map<int,int> &resultNodes,map<int,int>&resultLinks,SubstrateNetwork &substrateNetwork){
//虚拟链路扩张因子的计算
//虚拟链路扩张因子表示:虚拟网络中所有虚拟链路的映射路径长度的平均值
    double sf =0;
    int vlink =0;
    for(auto iter=resultLinks.begin();iter!=resultLinks.end();iter++){
       cout<<iter->first<<'\t'<<iter->second<<endl;
       sf += substrateNetwork.getNodeLinkedBW(iter->first);
    }
}
void display_bytime(Request &request,map<int,int> &resultNodes,map<int,int>&resultLinks){
    cout<<request.arrival<<"  to  "<<request.duration<<endl;
    double acc=0;
    int sum_suc=0;
    for(int i=0;i<request.links;i++){
    	if(resultLinks.at(i)==request.mapLinks.at(i).linkId){
    	    sum_suc++;
    	}
    }
    acc = (double)sum_suc/request.links;
    cout<<"accuracy is: "<<acc<<endl;
}
void BFS_node_mapping(SubstrateNetwork &substrateNetwork,Request &request,map<int,int> &resultNodes,map<int,vector<int>>&resultLinks){
    int sNodeMapped[substrateNetwork.nodes]={0};
    int reqNodeMapped[request.nodes]={0};
    int reqQue[request.nodes];
    int sQue[substrateNetwork.nodes];
    resultNodes.clear();
    resultLinks.clear();
    int reqhead = 0;
    int reqrear = 0;
    int sqhead  = 0;
    int sqrear  = 0;
    int w,q;
    for (int i = 0; i < request.nodes; ++i){
        Node p;
        if(reqNodeMapped[request.mapNodes.at(i).nodeId]==0){
            p = substrateNetwork.mapNodes.at(i);
            if(p.CPU > request.mapNodes.at(i).CPU&&(reqNodeMapped[p.nodeId])){
                resultNodes.insert(pair<int,int>(request.mapNodes.at(i).nodeId,p.nodeId));
//                mappinglinks
                reqNodeMapped[request.mapNodes.at(i).nodeId]=1;
                sNodeMapped[p.nodeId]=1;
                reqQue[reqrear++]=request.mapNodes.at(i).nodeId;
                sQue[sqrear++] = p.nodeId;
            }
            while(reqhead!=reqrear){
                int u = reqQue[reqhead++];//u是nodeid  不是序号,因为此时节点顺序已经排序完毕
                int p = sQue[sqhead++];
                q = substrateNetwork.first_vertex(p);//返回第一个邻接点的序号,也是nodeid；
                for(w=request.first_vertex(u);w>=0;w=request.next_vertix(u,w)){
                    if(!reqNodeMapped[w]&&!sNodeMapped[q]){
                        if((substrateNetwork.getnodebyId(q).residual_cpu > request.getnodebyId(w).CPU)
//                        &&(substrateNetwork.getbandwidth(p,q)>request.getbandwidth(u,w))
                                ){
                            resultNodes.insert(pair<int,int>(w,q));
                            reqQue[reqrear++]= w;
                            sQue[sqrear++]   = q;
                            cout<<"req "<<w<<"sq "<<q<<endl;
                            reqNodeMapped[w]=1;
                            sNodeMapped[q]  =1;
//                            cout<<"while \n";
                        }
                    }
                    q=substrateNetwork.next_vertix(p,q);
                    if(q == -1){
                        break;
                    }
//                    cout<<"id = "<<w<<" for\n"<<endl;
                }
            }
        }
    }
    for(int i=0;i<request.links;i++){
        int subfrom,subto;
        for(auto iter=resultNodes.begin();iter!=resultNodes.end();iter++){
            if(request.mapLinks.at(i).from==iter->first)
                subfrom=iter->second;
        }
        for(auto  iter=resultNodes.begin();iter!=resultNodes.end();iter++){
            if(request.mapLinks.at(i).to==iter->first)
                subto=iter->second;
        }
        resultLinks.insert(pair<int,int>(subfrom,subto));
    }
}
//根据节点效率，实现思路：先判断节点状态，公式为
//(Power_max-Power_base)*被映射虚拟节点需要的cpu/物理节点所持有的CPU资源;
bool BFS_NodeMapping(SubstrateNetwork &substrateNetwork,Request &request,CopySub &copySub,map<int,int> &resultnodes,map<int, vector<int>> &resultlinks){
    map<int,bool> sNodeMapped;
    for(int i=0;i<substrateNetwork.nodes;i++) sNodeMapped[i]=false;
    map<int,bool>reqNodeMapped;
    for(int i=0;i<request.nodes;i++) reqNodeMapped[i]=true;

    set<int> setMapped;//已经映射的SN节点集合
    setMapped.clear();//用于计算未来映射的节点和已经映射节点集合距离时使用。
    //进行bfs
    int reqQue[request.nodes]; //虚拟节点队列
    int sQue[substrateNetwork.nodes];//向后查找的物理节点
    resultnodes.clear();
    resultlinks.clear();
    int reqhead = 0;
    int reqrear = 0;
    int sqhead  = 0;
    int sqrear  = 0;
    int w,q;
    for (int i = 0; i < request.nodes; ++i) {
        Node p;
        cout << reqNodeMapped[i] <<' ';
        if (reqNodeMapped[request.mapNodes[i].nodeId]) {
            p = substrateNetwork.mapNodes[i];
            if (p.residual_cpu > request.mapNodes[i].CPU && (reqNodeMapped[p.nodeId])) {
                resultnodes.insert(pair<int, int>(request.mapNodes.at(i).nodeId, p.nodeId));
                //节点映射
                reqNodeMapped[request.mapNodes.at(i).nodeId]=1;
                sNodeMapped[p.nodeId] = 1;
                reqQue[reqrear++] = request.mapNodes.at(i).nodeId;
                sQue[sqrear++] = p.nodeId;
            }
            while (reqhead != reqrear) {
                int u = reqQue[reqhead++];//u是nodeid  不是序号,因为此时节点顺序已经排序完毕
                int p = sQue[sqhead++];
                q = substrateNetwork.first_vertex(p);//返回第一个邻接点的序号,也是nodeid；
                for (w = request.first_vertex(u); w >= 0; w = request.next_vertix(u, w)) {
                    if (!sNodeMapped[q]) {
                        if ((substrateNetwork.getnodebyId(q).residual_cpu > request.getnodebyId(w).CPU)
//                        &&(substrateNetwork.getbandwidth(p,q)>request.getbandwidth(u,w))
                                ) {
                            resultnodes.insert(pair<int, int>(w, q));
                            reqQue[reqrear++] = w;
                            sQue[sqrear++] = q;
                            cout << "req " << w << "sq " << q << endl;
//                            reqNodeMapped[w]=1;
                            sNodeMapped[q] = 1;
//                            cout<<"while \n";
                        }
                    }
                    q = substrateNetwork.next_vertix(p, q);
                    if (q == -1) {
                        continue;
                    }
//                    cout<<"id = "<<w<<" for\n"<<endl;
                }
            }
//        }
        }
    }

    //----
//    map<int,bool> sNodeMapped;
//    for(int i=0;i<substrateNetwork.nodes;i++) sNodeMapped[i]=false;
//    map<int,bool>reqNodeMapped;
//    for(int i=0;i<request.nodes;i++) reqNodeMapped[i]=false;
//
//    set<int> setMapped;//已经映射的SN节点集合
//    setMapped.clear();//用于计算未来映射的节点和已经映射节点集合距离时使用。
//
//
//    int reqQue[request.nodes];
//    int sQue[substrateNetwork.nodes];
//    resultnodes.clear();
//    resultlinks.clear();
//    int reqhead = 0;
//    int reqrear = 0;
//    int sqhead  = 0;
//    int sqrear  = 0;
//    int w,q;
//    for (int i = 0; i < request.nodes; ++i){
//        Node p;
//        if(!reqNodeMapped[request.mapNodes[i].nodeId]){
//            p = substrateNetwork.mapNodes.at(i);
//            if(p.CPU > request.mapNodes.at(i).CPU&&(reqNodeMapped[p.nodeId])){
//                resultnodes.insert(pair<int,int>(request.mapNodes.at(i).nodeId,p.nodeId));
////                mappinglinks
//                reqNodeMapped[request.mapNodes.at(i).nodeId]=1;
//                sNodeMapped[p.nodeId]=1;
//                reqQue[reqrear++]=request.mapNodes.at(i).nodeId;
//                sQue[sqrear++] = p.nodeId;
//            }
//            while(reqhead!=reqrear){
//                int u = reqQue[reqhead++];//u是nodeid  不是序号,因为此时节点顺序已经排序完毕
//                int p = sQue[sqhead++];
//                q = substrateNetwork.first_vertex(p);//返回第一个邻接点的序号,也是nodeid；
//                for(w=request.first_vertex(u);w>=0;w=request.next_vertix(u,w)){
//                    if(!reqNodeMapped[w]&&!sNodeMapped[q]){
//                        if((substrateNetwork.getnodebyId(q).residual_cpu > request.getnodebyId(w).CPU)
////                        &&(substrateNetwork.getbandwidth(p,q)>request.getbandwidth(u,w))
//                                ){
//                            resultnodes.insert(pair<int,int>(w,q));
//                            reqQue[reqrear++]= w;
//                            sQue[sqrear++]   = q;
//                            cout<<"req "<<w<<"sq "<<q<<endl;
//                            reqNodeMapped[w]=1;
//                            sNodeMapped[q]  =1;
////                            cout<<"while \n";
//                        }
//                    }
//                    q=substrateNetwork.next_vertix(p,q);
//                    if(q == -1){
//                        return false;
//                    }
////                    cout<<"id = "<<w<<" for\n"<<endl;
//                }
//            }
//        }
//    }
    //----
//    for(int i=0;i<request.nodes;i++){//虚拟网络也用H值最大的节点开始映射
//        double need_cpu=request.mapNodes[i].CPU;//虚拟节点所需求的CPU容量大小
//        vector<int> vecSN;//存放物理网络节点的向量
//        for(int k =0 ;k<substrateNetwork.nodes;k++){
//            vecSN.push_back(substrateNetwork.mapNodes[k].nodeId);//将物理节点按H值排序后的序号记录下来
//        }
//        for(int j=0;j<vecSN.size();j++){
//            if(!sNodeMapped[vecSN[j]]){
//                if(substrateNetwork.mapNodes[vecSN[j]].residual_cpu > request.mapNodes[i].CPU){
//                    resultnodes[i] = vecSN[j];//节点映射结果存放
//                    sNodeMapped[vecSN[j]] = true;//该物理节点映射完毕
//                    // sn.mapNodes[vecSN[j]].CPU-=request.mapNodes[request.mapNodes[i].nodeId].CPU;
//                    setMapped.insert(vecSN[j]);
//                    break;
//                }
//            }
//            if(j==substrateNetwork.nodes){//映射失败
//                return false;
//            }
//        }
//    }

}
int main()
{
    SubstrateNetwork substrateNetwork;
    CopySub copySub;
    char filesub[] = "../sub/sub.txt";
    substrateNetwork.initSubstrateNetwork(filesub);
    substrateNetwork.initneighbor();
    substrateNetwork.sortNodes();
    substrateNetwork.getShortest(copySub);//因为floyd现要On3,可以先把weight和path数组存起来，找最短路的时候直接取出。
    int acc = 0;
    for (int cnt = 0; cnt < 20; cnt++) {//进行循环输入request文件的路径，
        cout << "第" << cnt + 1 << "组网络请求\n";
        char reqfile[] = "../reqs0/req";
        sprintf(reqfile, "%s%d.txt", reqfile, cnt);
        Request request;
        request.initRequest(reqfile);
        request.initEachNodesNeighbor();
        request.sortNodes();
        //存放结果
        map<int,int> resultNodes;
        map<int,vector<int>>resultLinks;
        set<int> setMapped;
        BFS_NodeMapping(substrateNetwork,request,copySub,resultNodes,resultLinks);
        printf(acc/(cnt+1));
    }
    return 0;
}
