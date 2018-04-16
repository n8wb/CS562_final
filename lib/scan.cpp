#include "scan.h"

using namespace std;


vector<struct ct>* get_ct(vector<string>* select_columns, vector<Column*>* columns){
	vector<struct ct>* cols = new vector<struct ct>();
	for(vector<string>::iterator it = select_columns->begin();it!=select_columns->end();it++){
		struct ct c;
		c.name = *it;
		for(vector<Column*>::iterator jt = columns->begin();jt!=columns->end();jt++){
			if(c.name.find((*jt)->name)!=string::npos){
				c.type = (*jt)->type;
				break;
			}
		}
		cols->push_back(c);
	}
	return cols;
}

string get_helpers(int no_grouping_vars, vector<string>* select_columns, vector<Column*>* columns){
	vector<struct ct>* cols = get_ct(select_columns,columns);
	string out;
	bool first = true;


	for(int i = 1;i <= no_grouping_vars;i++){
		out += "size_t vfind"+ itoa(i) +"(vector<struct group"+itoa(i)+"*>* data ";
		string vfind_arg;
		string vfind_comp;
		bool first = true;
		for(vector<struct ct>::iterator it = cols->begin(); it != cols->end(); it++){
			vfind_arg = ", " + Column::getOutCppType(it->type) + " " + it->name;
			if(first){
				first = false;
			}else{
				vfind_comp += " && ";
			}
			switch(it->type){
				case CPP_STRING:
					vfind_comp += it->name +".compare((*it)->"+ it->name +") == 0";
					break;
				case CPP_INT64:
				case CPP_FLOAT:
				case CPP_DOUBLE:
					vfind_comp += it->name + " == " + "(*it)->" + it->name;
					break;
				case CPP_DATE:
				case CPP_RAW:
				case CPP_VECTOR:
					throw "Date, Raw, and Vector C++ types are not supported!";
					break;
			}
		}
		out += vfind_arg + "){\n"
			"\t size_t index = 0;\n"
			"\tfor(vector<struct group"+itoa(i)+"*>::iterator it = data->begin(); it != data->end(); it++){\n";
		out +=  "\t\tif("+vfind_comp+"){\n";
		out +=		"\t\t\treturn index;\n"
			    "\t\t}\n"
			    "\t\tindex++;\n"
			"\t}\n"
			"\t return -1;\n"
			"}\n";
	}
	
	out += "bool is_unique(vector<struct sales*>* data,struct sales* entry){\n"
		"\tfor(vector<struct sales*>::iterator it = data->begin();it != data->end();it++){\n";
	
	out += "\t\tif(";
	for(vector<struct ct>::iterator it = cols->begin();it != cols->end(); it++){
		if(first){
			first = false;
		}else{
			out += " && "; 
		}
		switch(it->type){
			case CPP_STRING:
				out += "entry->"+ it->name +".compare((*it)->"+ it->name +") == 0";
				break;
			case CPP_INT64:
			case CPP_FLOAT:
			case CPP_DOUBLE:
				out += "entry->"+ it->name + " == " + "(*it)->" + it->name;
				break;
			case CPP_DATE:
			case CPP_RAW:
			case CPP_VECTOR:
				throw "Date, Raw, and Vector C++ types are not supported!";
				break;
		}
	}
	out += "){\n"
			"\t\t\treturn false;\n"
		"\t\t}\n"
		"\t}\n"
		"\treturn true;\n"
		"}\n"
	    "vector<struct sales*>* get_uniques(vector<struct sales*>* data){\n"
			"\tvector<struct sales*>* out = new vector<struct sales*>();\n"
			"\tfor(vector<struct sales*>::iterator it = data->begin();it != data->end();it++){\n"
				"\t\tif(is_unique(out,*it)){\n"
					"\t\t\tout->push_back(*it);\n"
				"\t\t}\n"
			"\t}\n"
			"\treturn out;\n"
		"}\n";
	return out;
}


/*x handle_avg(){
	struct avg_tuple{
		map<string,int:string> tuple
		int count

	};
}
*/

string create_scans(int no_grouping_vars,vector<Column*>* columns,vector<string>* select_columns,vector<Aggregate*>* select_aggregates,
				    vector<Aggregate*>* all_aggregates){
	//Initial scan to fill unique non-aggregates
	
	string out = "\tvector<struct sales*>* uniques = get_uniques(data);\n"
				 "\tfor(vector<struct sales*>::iterator it = uniques->begin();it != uniques->end();it++){\n"
				 "\t\tstruct mf_structure* entry = (struct mf_structure*)calloc(1,sizeof(struct mf_structure));\n";
	for(vector<string>::iterator it = select_columns->begin(); it != select_columns->end(); it++){
		out += "\t\tentry->"+ *it + " = "+ "uniques->" + *it + ";\n";
		out += "\t\tmf_struct->push_back(entry);\n";
	}
	for(int i = 1;i <= no_grouping_vars;i++){
		string datatype = "struct group"+itoa(i);
		string var = "entry"+itoa(i);
		out += "\t\t"+datatype + "* "+ var + " = (" + datatype + "*)calloc(1,sizeof("+datatype+"));\n";
		for(vector<string>::iterator it = select_columns->begin(); it != select_columns->end(); it++){
			out += "\t\t"+ var +"->"+ *it + " = "+ "uniques->" + *it + ";\n";
			out += "\t\tgroup"+ itoa(i) +"->push_back("+ var +");\n";
		}
	}
	out += "\t}\n";
	//End initial scan
	//At this point, the mf_struct and the grouping variable tables are initialized with all the unique grouping id
	string structures;
	vector<struct ct>* cols = get_ct(select_columns,columns);
	//Create the rest of the scans
	for(int i = 1; i <= no_grouping_vars; i++){

	}
	return out;
}