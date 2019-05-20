//
// Created by waynamigo on 18-12-5.
//
#include "Substrate.h"
#include "Request.h"
#include "classic_tools.h"
int main(){
    SubstrateNetwork substrateNetwork,tempsubstrateNetwork;//这里的temp是为了防止情况：节点映射操作成功并分配完资源后，链路映射失败，回退物理网络情况
    CopySub copySub;
    char filesub[] = "../sub/sub.txt";
    //sortNodes 1表示经典按H值排序的算法，2表示加入负载均衡系数后进行排序的方法
    substrateNetwork.initSubstrateNetwork(filesub);
    substrateNetwork.sortNodes(1);//从文件初始化  第一次排序
    int acc = 0;
    substrateNetwork.getShortest(copySub);//因为floyd复杂度 On3,先存一下path，找最短路的时候直接取。
    copySub.print_copy();

    bool MapFlag,BackFlag =false;//当前节点队列是否全部映射，有一个节点失败，返回false
    for (int cnt = 0; cnt < 30; cnt++) {//进行循环输入request文件的路径，已经设置好了，改变cnt上限
        cout<<"第"<<cnt+1<<"组网络请求\n";
        char reqfile[] ="../reqs0/req";
        sprintf(reqfile,"%s%d.txt",reqfile, cnt);
        tempsubstrateNetwork = substrateNetwork;
        substrateNetwork.sortNodes(2);//每一次应映射之后进行一次排序 参数1代表H值排序，2代表加入负载均衡系数后进行排序
//        tempsubstrateNetwork.sortNodes();
        Request request;
        request.initRequest(reqfile);
        request.sortNodes();//虚拟节点也按H值排序
        map<int,int> resultNodes;//(虚拟节点 ,物理节点)结果对应存放
        map<int, vector<int>> resultLinks;// 存放链路结果
        MapFlag = node_mapping(tempsubstrateNetwork, request, resultNodes);
        if (MapFlag){
            //分配资源
            cout<<"查找物理节点成功，分配资源"<<endl;
            //distribute node resource 分配资源
            MapFlag = Distri_nodes(tempsubstrateNetwork, request, resultNodes);

            if (MapFlag) {
                cout<<"节点分配资源成功，进行链路映射"<<endl;
                if(!resultNodes.empty()){
                    MapFlag = link_mapping(tempsubstrateNetwork,request,copySub,resultNodes,resultLinks);
                    cout<<"链路查找成功，分配资源"<<endl;
                    if(MapFlag){
                        Distri_links(tempsubstrateNetwork,request,resultLinks);
                        cout<<"本组request映射成功"<<endl;
                        acc++;
                    } else{
                        BackFlag=true;
                    }
                } else{
                    BackFlag =true;
                }
            } else{
                BackFlag=true;
            }
        }
        if(!BackFlag){//映射成功 不回退物理节点资源，给物理网络更新
            substrateNetwork = tempsubstrateNetwork;
        }
        //每一组request映射结束后输出一次接受率
        cout<<(double)acc/(cnt+1)<<endl;
    }
    return 0;
}