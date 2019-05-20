#ifndef SUBSTRATE_H_INCLUDED
#define SUBSTRATE_H_INCLUDED
#include "Base.h"
#define MAX_REQ_LINKS 100
class SubstrateNetwork{
public:
	int nodes;
	int links;
	double totalCPU;
	double totalBW;
	double power;
	map<int, Node> mapNodes;
	map<int, Link> mapLinks;
	vector<vector<int>> matrix;//无向图
	void initSubstrateNetwork(char *filepath);//从文件中初始化
	double getNodeLinkedBW(int nodeid);//用来排序的计算节点 bandwidth总和
	void sortNodes(int type);////按H值=cpu*sum(bandwidth)排序;
    void getShortest(CopySub &copySub);
    void displayShortest(CopySub &copySub,int from,int to);
    bool findShortestByVirtualFrom_To(CopySub &copySub,int from,int to,vector<int>&resultsubPath);
    int getLinkId(int start,int end);

    void initneighbor();
    void copySubNetwork(SubstrateNetwork &copysubstrateNetwork);
	void print();//查看结构体内容
    Node getnodebyId(int id);//按节点编号查找节点
    int first_vertex(int v){
        int i;
        if (v<0 || v>nodes)
            return -1;

        for (i = 0; i <=nodes; i++)
            if (matrix[v][i]!=0 && matrix[v][i]!=-1)
                return i;
        return -1;
    }
    int next_vertix(int v,int w){
        int i;
        if (v<0 || v>nodes || w<0 || w>nodes)
            return -1;
        for (i = w+1 ; i <= nodes; i++)
            if (matrix[v][i]!=0 && matrix[v][i]!=-1)
                return i;
        return -1;
    }
    virtual ~SubstrateNetwork();
};
int SubstrateNetwork::getLinkId(int start,int end){
    for(int i=0;i<links;i++){
        if(mapLinks.at(i).from == start&&mapLinks.at(i).to ==end){
            return mapLinks.at(i).linkId;
        }
    }
    return -1;
}
void SubstrateNetwork::getShortest(CopySub &copySub) {
    copySub.nodes = this->nodes;
    copySub.links = this->nodes;
    copySub.path.resize(this->nodes);
    copySub.weight.resize(this->nodes);
    for (int i = 0; i < nodes; ++i) {
        copySub.weight[i].resize(nodes,10000);
        copySub.path[i].resize(nodes,-1);
    }
    int from,to;
    for (int j = 0; j <links ; ++j) {
        from = mapLinks[j].from;
        to   = mapLinks[j].to;
        copySub.weight[from][to]  = mapLinks.at(j).bandwidth;
        copySub.weight[from][from]=0;
        copySub.weight[to][to]    =0;
        copySub.path[from][to]=from;
        copySub.path[from][from]=from;
        copySub.path[to][to] = to;
    }
    for(int k = 0;k < nodes;k++){
        for(int i= 0;i < nodes;i++){
            for(int j = 0;j < nodes;j++){
                if((copySub.weight[i][k] > 0 && copySub.weight[k][j] && copySub.weight[i][k] < 10000 && copySub.weight[k][j] < 10000) && (copySub.weight[i][k] + copySub.weight[k][j] < copySub.weight[i][j])){//前面一部分是防止加法溢出
                    copySub.weight[i][j] = copySub.weight[i][k] + copySub.weight[k][j];
                    copySub.path[i][j] = copySub.path[k][j];
                }
            }
        }
    }
}
//用来查看最短路（经过的节点）
void SubstrateNetwork::displayShortest(CopySub & copySub,int from, int to) {
    stack<int> shortpath;
    int temp = to;
    int cnt = nodes;
    while(temp != from){
        shortpath.push(temp);
        //  37 80,temp =54
        temp = copySub.path[from][temp];
        cnt --;
        if (cnt<=0){
            break;
        }
    }
    shortpath.push(from);
    cout<<"path:";
    while(!shortpath.empty()){
        cout<<shortpath.top()<<" ";
        shortpath.pop();
    }
    cout<<endl;
}
//map<int,int> &resultnodes,map<int, vector<int>> &resultlinks
bool SubstrateNetwork::findShortestByVirtualFrom_To(CopySub &copySub, int from, int to, vector<int> &resultsubPath){
    stack<int> shortpath;
    int temp = to;
    int cnt = nodes;
    while(temp != from){
        shortpath.push(temp);
        //  37 80,temp =54
        temp = copySub.path[from][temp];
        cnt--;
        if(cnt<=0){
            return false;
        }
    }
    shortpath.push(from);
    cout<<"bandwidth of path:"<<copySub.weight[from][to]<<endl;
    while(!shortpath.empty()){
        resultsubPath.push_back(shortpath.top());//save path
        shortpath.pop();
    }
    return true;
}
//存储节点的邻居节点和链路
void SubstrateNetwork::initneighbor(){
    //存储节点i发出的链路id
    for(int i=0;i<nodes;i++){
        for(int j=0;j<links;j++){
            if(mapLinks[j].from == i ||mapLinks[j].to==i){
                mapNodes[i].linkNeighbors.insert(mapLinks[j].linkId);
            }
        }
    }
    for(int j=0;j<links;j++){
        int from = mapLinks[j].from;
        int end   = mapLinks[j].to;
        mapNodes[from].nodeNeighbors.insert(end);
        mapNodes[end].nodeNeighbors.insert(from);
    }
}

SubstrateNetwork::~SubstrateNetwork() {
    mapLinks.clear();
    mapNodes.clear();
}
void SubstrateNetwork::copySubNetwork(SubstrateNetwork &copysubstrateNetwork){
        copysubstrateNetwork.nodes=this->nodes;
        copysubstrateNetwork.links=this->links;
//        vector<int> tempmatrix(nodes,-1);
//        copysubstrateNetwork.matrix.resize(nodes,tempmatrix);
//        copysubstrateNetwork.totalCPU=0;
//        copysubstrateNetwork.totalBW=0;
//        tempmatrix.clear();
//        Node tempnode;
//        Link templink;
//        for(int i=0;i<nodes;i++){
//            tempnode.x= mapNodes[i].x;
//            tempnode.y= mapNodes[i].y;
//            tempnode.residual_cpu = mapNodes[i].residual_cpu;
//            tempnode.CPU = mapNodes[i].CPU;
//            tempnode.nodeId=mapNodes[i].nodeId;//上一次排好的顺序id,不是等于i
//            tempnode.residual_cpu=tempnode.CPU;
//            tempnode.ratio=mapNodes[i].ratio;//节点未映射，一开始的ratio=0，后面占用资源时，ratio=residual_cpu/CPU
//            copysubstrateNetwork.totalCPU+=tempnode.CPU;
//            copysubstrateNetwork.mapNodes.insert(pair<int,Node>(i,tempnode));
//        }
//        for(int i=0;i<links;i++){
//            templink.from=mapLinks[i].from;
//            templink.to  =mapLinks[i].to;
//            templink.bandwidth = mapLinks[i].bandwidth;// the forth num is not used
//            templink.linkId=mapLinks[i].linkId;//边的序号从1开始 1 2 3 4 5
//            templink.residual_bw=mapLinks[i].residual_bw;
//            templink.ratio=mapLinks[i].ratio;
//            copysubstrateNetwork.totalBW+=templink.bandwidth;
//            copysubstrateNetwork.mapLinks.insert(pair<int,Link>(i,templink));
//            copysubstrateNetwork.matrix[templink.from][templink.to]=templink.bandwidth;
//            copysubstrateNetwork.matrix[templink.to][templink.from]=templink.bandwidth;
//        }
}
Node SubstrateNetwork::getnodebyId(int id){
    int i;
    for( i=0;i<nodes;i++){
        if(mapNodes.at(i).nodeId==id){
            return mapNodes.at(i);
        }
    }
}
//查看读取及排序后的节点，以及更新cpu和带宽后的数据
void SubstrateNetwork::print(){
    for(int i=0;i<nodes;i++){
        cout<<"nodeid"<< mapNodes[i].nodeId <<" residualcpu"<<mapNodes.at(i).residual_cpu<<"neighbor:"<<endl;
        auto it = mapNodes[i].nodeNeighbors.begin();
        while (it!=mapNodes[i].nodeNeighbors.end())
        {
            cout<<(*it)<< ' ';
            it++;
        }
        cout<<endl;
    }
}
//计算中心度
int get_Cent(SubstrateNetwork &substrateNetwork,int fromid){
    int centrality=0;
    for(int i=0;i<substrateNetwork.nodes;i++){
        centrality += substrateNetwork.mapNodes[i].hops;
        cout<<substrateNetwork.mapNodes[i].hops;
    }
    return centrality;
}
void SubstrateNetwork::initSubstrateNetwork(char *filepath){//初始化
    FILE *fp;
    fp=fopen(filepath,"r");
    if(fscanf(fp,"%d %d",&nodes,&links)){
        vector<int> tempmatrix(nodes,-1);//from common.h
        matrix.resize(nodes,tempmatrix);//之前实现CAA时直接在main中用的vector重新读取，还是在初始化时直接存比较简洁
        totalBW=0;
        totalCPU=0;
        tempmatrix.clear();
        Node tempnode;
        Link templink;
        for(int i=0;i<nodes;i++){
            fscanf(fp,"%d %d %lf",&tempnode.x,&tempnode.y,&tempnode.CPU);
            tempnode.nodeId=i;//=i+1;更改，物理网络的节点按文件中从上往下0号来存
            tempnode.residual_cpu=tempnode.CPU;
            tempnode.ratio=0;//节点未映射，一开始的ratio=0，后面占用资源时，ratio=residual_cpu/CPU
            totalCPU+=tempnode.CPU;
            mapNodes.insert(pair<int,Node>(i,tempnode));
        }
        for(int i=0;i<links;i++){
            double nouse;
            fscanf(fp,"%d %d %lf %lf",&templink.from,&templink.to,&templink.bandwidth,&nouse);// the forth num is not used
            templink.linkId=i;//边的序号从1开始 1 2 3 4 5
            templink.residual_bw=templink.bandwidth;
            templink.ratio=0;
            totalBW+=templink.bandwidth;
            mapLinks.insert(pair<int,Link>(i,templink));
            matrix.at(templink.from).at(templink.to)=templink.bandwidth;
            matrix[templink.to][templink.from]=templink.bandwidth;
        }
        fclose(fp);
    }
}
double SubstrateNetwork::getNodeLinkedBW(int nodeid){//计算节点bandwidth总和，用在最初的排序
    double centrality=0;
    for(int i=0;i<mapLinks.size();i++){
        if(mapLinks[i].from==nodeid){
            centrality+=mapLinks[i].bandwidth;
        }
    }
    for(int i=0;i<mapLinks.size();i++){
        if(mapLinks[i].to==nodeid){
            centrality+=mapLinks[i].bandwidth;
        }
    }
    return 1/centrality;
}

void SubstrateNetwork::sortNodes(int type){
    double H_cpubw[mapNodes.size()];
    //根据H值排序
    if(type==1){
        for(int i=0;i<mapNodes.size();i++){
            H_cpubw[i] = mapNodes[i].residual_cpu*getNodeLinkedBW(mapNodes[i].nodeId);
        }
    }

    //系数负载
    if(type==2){
        for(int i=0;i<mapNodes.size();i++){
            double H = mapNodes[i].residual_cpu*getNodeLinkedBW(mapNodes[i].nodeId);
            H_cpubw[i] = 0.5*H+0.2*mapNodes[i].ratio+0.3*mapNodes[i].hops;
        }
    }
    //冒泡
    for(int i=0;i<mapNodes.size()-1;i++){
        for(int j=0;j<mapNodes.size()-1-i;j++){
            if(H_cpubw[j]<H_cpubw[j+1]){
                double tmpH_cpubw=H_cpubw[j];
                H_cpubw[j]=H_cpubw[j+1];
                H_cpubw[j+1]=tmpH_cpubw;
                Node node = mapNodes[j];
                mapNodes[j]=mapNodes[j+1];
                mapNodes[j+1]=node;
            }
        }
    }
}
#endif // SUBSTRATE_H_INCLUDED
