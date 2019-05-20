#ifndef REQUEST_H_INCLUDED
#define REQUEST_H_INCLUDED
#include "Base.h"
#define MAX_REQ_LINKS 100
struct Request{
    int nodes;//节点数
    int links;//链路数
    int arrival;//起始时间
    int duration;//持续时间
    map<int,Node> mapNodes;//由于数据文件中从0号节点开始读取，map的key值是节点（链路）的索引号，方便存取
	map<int,Link> mapLinks;
	vector<vector<int>> matrix;//矩阵表示网络
	void initRequest(char *filepath);//从一个req.txt文件读取，初始化一个Request
	int iflinked(int from,int to); //计算虚拟链路扩张因子的vlink参数
	Node getnodebyId(int id);//获取一个Node
    double getReq();//计算收益
    double getCost(double **shortestpaths,set<int>&setMappedSN);//计算开销
    int hops(double **shortestpaths,set<int>&setMappedSN,int start);//计算跃点数hops
	double getNodeLinkedBW(int nodeid);
	void sortNodes();//按H值=cpu*sum(bandwidth)排序;
	void print();//用来看节点排序情况
    void initEachNodesNeighbor(){
        for(int i=0;i<links;i++){
            int start = mapLinks[i].from;
            int end = mapLinks[i].to;
            mapNodes[start].nodeNeighbors.insert(end);
            mapNodes[end].nodeNeighbors.insert(start);
        }
    }
	double getR_C(double **shortestpath,set<int>&setMapped){//计算收益开销比
        return 1.0*(getCost(shortestpath,setMapped)/getReq());
	}
	//bfs
	int first_vertex(int v){
        int i;
        if (v<0||v>nodes)
            return -1;
        for (i = 0; i <nodes; i++)
            if (matrix[v][i]!=-1)
                return i;
        return -1;
    }
    int next_vertix(int v,int w){
        int i;
        if (v<0 || v>nodes || w<0 || w>nodes)
            return -1;
        for (i = w+1 ; i < nodes; i++)
            if (matrix[v][i]!=-1)
                return i;
        return -1;
    }
};
void Request::print(){//用来看节点排序情况
    cout<<"用来看节点排序和矩阵情况"<<endl;
    for(int i=0;i<nodes;i++){
            cout<<mapNodes.at(i).nodeId<<"  ";
    }
    cout<<endl;
    for(int i=0;i<nodes;i++){
        for(int j=0;j<nodes;j++){
            cout<<matrix[i][j]<<' ';
        }
        cout<<endl;
    }
}
double Request::getNodeLinkedBW(int nodeid){
    double sumbw=0;
    for(int i=0;i<mapLinks.size();i++){
        if(mapLinks[i].from==nodeid){
            sumbw+=mapLinks[i].bandwidth;
        }
    }
    for(int i=0;i<mapLinks.size();i++){
        if(mapLinks[i].to==nodeid){
            sumbw+=mapLinks[i].bandwidth;
        }
    }
    return sumbw;
}
int Request::hops(double **shortestpaths,set<int>&setMappedSN,int start){
    if(setMappedSN.empty()){
        return 1;//没有，则其为相邻节点，跳数为1
    }
    int minlenth=5;
    //遍历里面所有节点，找到两者距离最小的节点作为度量标准
    for(auto it=setMappedSN.begin();it!=setMappedSN.end();it++){
        if(shortestpaths[*it][start] < minlenth){
            minlenth=shortestpaths[*it][start];
        }
    }
    return minlenth;
}
double Request::getCost(double **shortestpaths,set<int>&setMappedSN){//计算开销
    double cpu=0,hops_bw=0;
    for(int i=0;i<nodes;i++){
        cpu+=mapNodes.at(i).CPU;
    }
    auto iter=setMappedSN.begin();
    while(iter!=setMappedSN.end()){
        hops_bw+=hops(shortestpaths,setMappedSN,(*iter));
        ++iter;
    }
    return cpu+hops_bw;
}
void Request::sortNodes(){
    double H_cpubw[mapNodes.size()];
    for(int i=0;i<mapNodes.size();i++){
        H_cpubw[i] = mapNodes[i].CPU*getNodeLinkedBW(mapNodes[i].nodeId);
    }
    //只普通的按H值排序，从大到小
    for(int i=0;i<mapNodes.size()-1;i++){
        for(int j=0;j<mapNodes.size()-1-i;j++){
            if(H_cpubw[j]<H_cpubw[j+1]){
                double tmpH_cpubw=H_cpubw[j];
                H_cpubw[j]=H_cpubw[j+1];
                H_cpubw[j+1]=tmpH_cpubw;
                Node node = mapNodes.at(j);
                mapNodes.at(j)=mapNodes.at(j+1);
                mapNodes.at(j+1)=node;
            }
        }
    }
}
//从一个request.txt文件中读取一个request结构体
void Request::initRequest(char *filepath){
    FILE *fp;
    fp=fopen(filepath,"r");
    int n1,n2,n3;
    //之前实现CAA时直接在main中用的vector重新读取，在初始化时直接存比较方便，目前没用到的数据读取掉
    if(fscanf(fp,"%d%d%d%d%d%d%d",&nodes,&links,&n1,&arrival,&duration,&n2,&n3)){
        vector<int> tempmatrix(nodes,-1);
        matrix.resize(nodes,tempmatrix);
        tempmatrix.clear();
        Node tempnode;
        Link templink;
        for(int i=0;i<nodes;i++){
            fscanf(fp,"%d %d %lf",&tempnode.x,&tempnode.y,&tempnode.CPU);
            tempnode.nodeId=i;//物理网络的节点按文件中从上往下0 1 2 3 4 5 号来存
            tempnode.residual_cpu=tempnode.CPU;
            tempnode.ratio=0;//虚拟节点，ratio不做操作
            mapNodes.insert(pair<int,Node>(i,tempnode));
        }
        double no_use;
        for(int i=0;i<links;i++){
            fscanf(fp,"%d %d %lf %lf",&templink.from,&templink.to,&templink.bandwidth,&no_use);
            templink.linkId=i;
            templink.residual_bw=templink.bandwidth;
            templink.ratio=0;
            mapLinks.insert(pair<int,Link>(i,templink));
            //初始化带宽
            matrix[templink.from][templink.to]=templink.bandwidth;
            matrix[templink.to][templink.from]=templink.bandwidth;
        }
        fclose(fp);
    }
}
double Request::getReq(){//获取request的cpu及链路需求bandwidth
    double cpu=0,bw=0;
    for(int i=0;i<nodes;i++){
        cpu+=mapNodes.at(i).CPU;
    }
    for(int i=0;i<links;i++){
        bw+=mapLinks.at(i).bandwidth;
    }
    return cpu+bw;
}
Node Request::getnodebyId(int id){
    int i;
    for( i=0;i<nodes;i++){
        if(mapNodes.at(i).nodeId==id){
            return mapNodes.at(i);
        }
    }
}
int Request::iflinked(int from, int to) {
    return 0;
}
#endif // REQUEST_H_INCLUDED
