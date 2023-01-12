#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "bvh_parser.h"
#include <stack>
using namespace std;

// a naive bvh parser

// 将HIERARCHY部分的数据存放入对应joint结构体的成员中
void LoadHierarchy(ifstream &file, joint &root);
// 将MOTION部分的数据存放入对应joint结构体的motion成员中和META结构体中
void LoadMotion(ifstream &file, joint &root, META &meta_data);

int main(int argc, char **argv)
{
    joint root;
    META meta_data;
    ifstream file(argv[1]);
    // do something
    if (!file.is_open())
    {
        cout << "Fail to open the file: " << argv[1] << endl;
        return -1;
    }
    LoadHierarchy(file, root);
    LoadMotion(file, root, meta_data);
    jsonify(root, meta_data);
    file.close();
    return 0;
}

void LoadHierarchy(ifstream &file, joint &root)
{
    joint *t = &root;
    stack<joint *> s;
    s.push(t);
    string oneline;
    while (getline(file, oneline))
    {
        string onechar;
        stringstream ss(oneline);
        ss >> onechar;
        
        if (onechar == "ROOT")
        {
            t->joint_type = "ROOT";
            ss >> t->name;
        }
        else if (onechar == "JOINT")
        {
            joint *child = new joint;
            s.push(child);
            t->children.push_back(child);
            t = child;
            t->joint_type = "JOINT";
            ss >> t->name;
        }
        else if (onechar == "End")
        {
            joint *child = new joint;
            child->joint_type = "End";
            joint *p = s.top();
            child->name = p->name + "_End";
            t->children.push_back(child);
            s.push(child);
            t = child;
        }
        else if (onechar == "}")
        {
            if (!s.empty())
            {
                s.pop();
                if (!s.empty())
                    t = s.top();
            }
        }
        else if (onechar == "OFFSET")
        {
            ss >> t->offset_x;
            ss >> t->offset_y;
            ss >> t->offset_z;
        }
        else if (onechar == "CHANNELS")
        {
            int channel_number;
            ss >> channel_number;
            for (int i = 0; i < channel_number; i++)
            {
                string channel_type;
                ss >> channel_type;
                t->channels.push_back(channel_type);
            }
        }
        else if (onechar == "MOTION")
        {
            break;
        }
        
    }
}


void LoadMotion(ifstream &file, joint &root, META &meta_data)
{
    string l_frame, s_frame;
    string l_frame_time, s_frame_time;
    getline(file, l_frame);
    getline(file, l_frame_time);
    stringstream ss_frame(l_frame);
    for(int i = 0; i < 2; i++)
    {
        if(i == 1) ss_frame >> meta_data.frame;
        else ss_frame >> s_frame;
    }
    stringstream ss_frame_time(l_frame_time);
    for(int i = 0; i < 3; i++)
    {
        if(i == 2) ss_frame_time >> meta_data.frame_time;
        else ss_frame_time >> s_frame_time;
    }

    string motion_once;                // 利用getline得到一行数据，存入字符串motion_once中
    while (getline(file, motion_once)) // 循环次数为frames的数值
    {
        stringstream ss(motion_once); // 利用stringstream流分隔数据，并实现数据类型转化
        joint *t = &root;
        stack<joint *> s; // 工作栈
        s.push(t);        // 根节点先入栈
        while (!s.empty())
        {
            // 取栈顶joint，出栈
            joint *p = s.top();
            s.pop();
            // 将当前joint的所有孩子倒序入栈
            int i;
            int children_number = p->children.size();
            for (i = children_number - 1; i >= 0; i--)
                s.push(p->children[i]);
            // 将与当前joint通道个数相同数量的数据push进vector中，此vector最后push进motion中
            int j;
            double data;
            int channel_number = p->channels.size();
            vector<double> motion_data;
            for (j = 0; j < channel_number; j++)
            {
                ss >> data;
                motion_data.push_back(data);
            }
            p->motion.push_back(motion_data);
        }
    }
}
