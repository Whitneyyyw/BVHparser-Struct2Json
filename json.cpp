#include<iostream>
#include<fstream>
#include"bvh_parser.h"
#include<cstring>
#include"cJSON.h"
using std::ofstream;

cJSON* CreateMeta(META &meta_data, cJSON *json)
{
	//frame
	cJSON_AddNumberToObject(json,"frame",meta_data.frame);
	//frame_time
	cJSON_AddNumberToObject(json,"frame_time",meta_data.frame_time);
	return json;
}

cJSON* CreateJoint(joint &root, cJSON *json_joint, int frame_number)
{
	joint *t = &root;
	int channel_number = t->channels.size();
	int children_number = t->children.size();
	int i, j;
	//type
	cJSON_AddStringToObject(json_joint,"type",t->joint_type.c_str());
	//name
	cJSON_AddStringToObject(json_joint,"name",t->name.c_str());
	//offsets
	const double offset[3] = {t->offset_x, t->offset_y, t->offset_z};
	cJSON *joint_offset = cJSON_CreateDoubleArray(offset, 3);
	cJSON_AddItemToObject(json_joint, "offset", joint_offset);
	//channels
	if(channel_number == 6)
	{
		const char* channel[6] = {t->channels[0].c_str(),t->channels[1].c_str(),t->channels[2].c_str(),t->channels[3].c_str(),t->channels[4].c_str(),t->channels[5].c_str()};
		cJSON *joint_channel = cJSON_CreateStringArray(channel, 6);
		cJSON_AddItemToObject(json_joint, "channels", joint_channel);

	} 
	else if(channel_number == 3)
	{
		const char* channel[3] = {t->channels[0].c_str(),t->channels[1].c_str(),t->channels[2].c_str()};
        //string->const char*,using .c_str()
		cJSON *joint_channel = cJSON_CreateStringArray(channel, 3);
		cJSON_AddItemToObject(json_joint, "channels", joint_channel);
	}
	else
	{
		cJSON *joint_channel = cJSON_CreateArray();
		cJSON_AddItemToObject(json_joint, "channels", joint_channel);
	}
	//motion
	cJSON *joint_motion = cJSON_CreateArray();
	for(i = 0; i < frame_number; i++)
	{
		// cJSON_AddNullToObject(json_joint,'\n');
		if(channel_number == 6)
		{
			const double obj[6] = {t->motion[i][0],t->motion[i][1],t->motion[i][2],t->motion[i][3],t->motion[i][4],t->motion[i][5]};
			cJSON *motion_obj = cJSON_CreateDoubleArray(obj, 6);
			cJSON_AddItemToArray(joint_motion, motion_obj);
		}
		else if(channel_number == 3)
		{
			const double obj[3] = {t->motion[i][0],t->motion[i][1],t->motion[i][2]};
			cJSON *motion_obj = cJSON_CreateDoubleArray(obj, 3);
			cJSON_AddItemToArray(joint_motion, motion_obj);
		}
		else
		{
			cJSON *motion_obj = cJSON_CreateArray();
			cJSON_AddItemToArray(joint_motion, motion_obj);
		}
	}
	cJSON_AddItemToObject(json_joint,"motion",joint_motion);
	//children
	cJSON *child_joint_array = cJSON_CreateArray();
	if(t->joint_type == "End")
	{
		cJSON_AddItemToObject(json_joint,"children",child_joint_array);
		return json_joint;
	}
	joint *p;
	for(i = 0; i < children_number; i++)
	{
		p = t->children[i];
		cJSON *child_joint = cJSON_CreateObject();
		child_joint = CreateJoint(*p, child_joint, frame_number);
        cJSON_AddItemToArray(child_joint_array, child_joint);
	}
	cJSON_AddItemToObject(json_joint,"children",child_joint_array);
	return json_joint;
}

cJSON* Struct2Json(joint &root, META &meta_data)
{
	cJSON *json = cJSON_CreateObject();
	json = CreateMeta(meta_data, json);
	cJSON *json_joint = cJSON_CreateObject();
	json_joint = CreateJoint(root, json_joint, meta_data.frame);
	cJSON_AddItemToObject(json,"joint",json_joint);
	return json;
}

void jsonify(joint root, META meta_data) {
    ofstream outfile;
    outfile.open("output.json");
    
    // output the root and meta_data
	if(!outfile.is_open())
	{
		std::cout << "Fail to open the file!" << std::endl;
		return;
	}
	cJSON *json = Struct2Json(root, meta_data);
    
	char *out = cJSON_Print(json);
	outfile << out;
	cJSON_Delete(json);
	free(out);

    outfile.close();
}

