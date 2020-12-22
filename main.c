//null 0
//number 1
//string 2
//bool 3
//obj 4
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>







char* new_str(char* str){
    char* ret=(char*)malloc(sizeof(char)*(strlen(str)+1));
    strcpy(ret,str);
    return ret;
}

typedef struct key{
    int start;
    int end;
}key;

void print_key(key* k){
    if(k==NULL) {
        printf("key: null\n\r");
        return;
    }
    printf("key{start:%d,end:%d}\n\r",k->start,k->end);
}

key* new_key(int start,int end){
    key* ret=(key*)malloc(sizeof(key));
    ret->start=start;
    ret->end=end;
    return ret;
}

void free_key(key* this){
    free(this);
}

typedef struct{
    int start;
    int end;
    int type;
}val;

void print_val(val* v){
    if(v==NULL){
        printf("null\n\r");
        return;
    }
    printf("val{start:%d,end:%d}\n\r",v->start,v->end);
}

val* new_val(int start,int end,int type){
    val* ret=(val*)malloc(sizeof(val));
    ret->start=start;
    ret->end=end;
    ret->type=type;
    return ret;
}
void free_val(val* this){
    free(this);
}

typedef struct json_obj{
    struct json_obj* pre;
    struct json_obj* next;
    char* json;
    key* item_key;
    val* item_val;
}json_obj;

void print_json_obj(json_obj* obj){
    if(obj==NULL){
        printf("null\n\r");
        return;
    }
    printf("key:{start:%d,end:%d}\n\r",obj->item_key->start,obj->item_key->end);

    char* json_type[]={"null","number","string","bool","obj"};

    printf("val:{start:%d,end:%d,type:%s}\n\r",obj->item_val->start,obj->item_val->end,json_type[obj->item_val->type]);
}


json_obj* new_json_obj(char* json){
    json_obj* obj=(json_obj*)malloc(sizeof(json_obj));
    obj->item_val=NULL;
    obj->item_key=NULL;
    obj->pre=NULL;
    obj->next=NULL;
    obj->json=new_str(json);
    return obj;
}
void free_json_obj(json_obj* this){
    json_obj* head=this;

    json_obj* p=head;
    while(p!=NULL){
        json_obj* next=p->next;
        free(p->json);
        free(p->item_val);
        free(p->item_key);
        free(p);
        p=next;
    }
}
int skip_space(char* json,int start,int end){
    while(start<end&&json[start]==' '){
        start++;
    }
    return start;
}


key* parse_key(char* json,int start,int end){
    start=skip_space(json,start,end);
    if(start==-1||start==end) return NULL;
    if(json[start]!='\''&&json[start]!='"') return NULL;
    char head=json[start];
    int _end=start+1;
    do{
        if(json[_end]==head){
            return new_key(start,_end);
        }
        if(json[_end]=='\\'){
            _end+=2;
            continue;
        }
        _end++;
    }while(_end<end);


    return NULL;
}

int skip_gap(char* json,int start,int end){
    start=skip_space(json,start,end);
    if(json[start]!=':') return -1;
    start=skip_space(json,start+1,end);
    return start;
}
int parse_obj(char* json,int start,int end){
    int left=1;
    while(start<end){
        if(json[start]=='{') left++;
        else if(json[start]=='}') left--;
        if(left==0) return start;
        start++;
    }
    return -1;
}
int parse_num(char* json,int start,int end){
    while(start<end&&json[start]<='9'&&json[start]>='0'){
        start++;
    }
    return start;
}


val* parse_val(char* json,int start,int end){
    if(json[start]=='{'){
        int _end=parse_obj(json,start+1,end);
        if(_end==-1) return NULL;
        return new_val(start,_end,4);
    }else if(json[start]<='9'&&json[start]>='0'){

        int end=parse_num(json,start,end);
        return new_val(start,end,1);
        //true
    }else if(json[start]=='t'&&json[start+1]=='r'&&json[start+2]=='u'&&json[start+3]=='e'){
        return new_val(start,start+3,3);
    }else if(json[start]=='f'&&json[start+1]=='a'&&json[start+2]=='l'&&json[start+3]=='s'&&json[start+4]=='e'){
        return new_val(start,start+4,3);
    }else if(json[start]=='\''||json[start]=='"'){
        key* k=parse_key(json,start,end);
        if(k==NULL) return NULL;

        val* ret=new_val(k->start,k->end,2);
        printf("parse_val.k->end:%d\n\r",ret->end);
        free(k);
        return ret;
    }else if(json[start]=='n'&&json[start+1]=='u'&&json[start+2]=='l'&&json[start+3]=='l'){
        return new_val(start,start+3,0);
    }else{
        return NULL;
    }
}

json_obj* parse_json(char* json,int start,int end){
    json_obj* pre_head=new_json_obj(json);
    json_obj* p=pre_head;


    start=skip_space(json,start,end);
    printf("parse_json.start: %d\n\r",start);
    if(json[start]!='{') return NULL;
    start++;
    while(start<end){
        key* k=parse_key(json,start,end);
        printf("parse_json.k  ");print_key(k);
        if(k==NULL) return NULL;
        start=k->end+1;
        start=skip_gap(json,start,end);
        printf("parse_json.start: %d\n\r",start);

        if(start>=end) return NULL;
        val* v=parse_val(json,start,end);
        printf("parse_json.v  ");print_val(v);
        if(v==NULL) return NULL;
        start=v->end+1;
        json_obj* next=new_json_obj(json);
        next->item_key=k;
        next->item_val=v;
        p->next=next;
        next->pre=p;
        p=p->next;

        start=skip_space(json,start,end);
        if(json[start]=='}'){
            printf("parse_obj.pre_head->next: ");print_json_obj(pre_head->next);
            return pre_head->next;
        }
        if(json[start]!=','){
            return NULL;
        }
        start++;
    }
    return pre_head->next;
}





int main() {
    char* json="{'Hello':{'name':'lcb'},'clang':99 }";
    int end=strlen(json);
    printf("main.end: %d\n\r",end);
    int start=0;
    json_obj* obj=parse_json(json,start,end);

    printf("main: ");print_json_obj(obj->next);
    free_json_obj(obj);
}
