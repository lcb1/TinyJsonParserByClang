#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    int start;
    int end;
}segment;

segment* new_segment(int start,int end){
    segment* seg=(segment*)malloc(sizeof(segment));
    seg->start=start;
    seg->end=end;
    return seg;
}

void free_seg(segment* this){
    free(this);
}

typedef struct {
    segment* seg;
}key;

key* new_key(segment* seg){
    key* k=(key*)malloc(sizeof(key));
    k->seg=seg;
    return k;
}
void free_key(key* this){
    free_seg(this->seg);
    free(this);
}

typedef struct {
    segment* seg;
    int type;
}val;

val* new_val(segment* seg,int type){
    val* v=(val*)malloc(sizeof(val));
    v->seg=seg;
    v->type=type;
    return v;
}
void free_val(val* this){
    free_seg(this->seg);
    free(this);
}


typedef struct json_obj{
    const char* json;
    struct json_obj* pre;
    struct json_obj* next;
    key* item_key;
    val* item_val;
}json_obj;

json_obj* new_json_obj(const char* json){
    json_obj* obj=(json_obj*)malloc(sizeof(json_obj));
    obj->json=json;
    return obj;
}

void free_json_obj(json_obj* this){
    while(this->pre!=NULL){
        this=this->pre;
    }
    while(this!=NULL){
        json_obj* next=this->next;
        free_json_obj(this->next);
        free_key(this->item_key);
        free_val(this->item_val);
        free(this);
        this=next;
    }
}

segment* parse_null(char* json,int start,int end){
    if(start>=end) return NULL;
    if(start+3<end&&json[start]=='n'&&json[start+1]=='u'&&json[start+2]=='l'&&json[start+3]=='l'){
        return new_segment(start,start+3);
    }
    return NULL;
}

int parse_pure_num(char* json,int start,int end){
    int p=start;
    while(p<end&&json[p]<='9'&&json[p]>='0'){
        p++;
    }
    return p;
}

segment* parse_num(char* json,int start,int end){
    if(start>=end) return NULL;
    int p1=start;
    if(json[p1]=='+'||json[p1]=='-'){
        p1++;
    }
    int p2=parse_pure_num(json,p1,end);
    if(p1==p2)return NULL;
    if(p2-p1>1&&json[p1]=='0') return NULL;
    if(json[p2]=='.'){
        int p3=parse_pure_num(json,p2+1,end);
        if(p3==p2+1) return NULL;
        p2=p3;
    }
    return new_segment(start,p2-1);
}


segment* parse_str(char* json,int start,int end){
    if(start>=end) return NULL;
    if(json[start]!='\''&&json[start]!='"') return NULL;
    char head=json[start];
    int _end=start+1;
    do{
        if(json[_end]==head){
            return new_segment(start,_end);
        }
        if(json[_end]=='\\'){
            _end++;
        }
        _end++;
    }while(_end<end);
    return NULL;
}


segment* parse_bool(char* json,int start,int end){
    if(start>=end) return NULL;
    if(start+3<end&&json[start]=='t'&&json[start+1]=='r'&&json[start+2]=='u'&&json[start+3]=='e'){
        return new_segment(start,start+3);
    }
    if(start+4<end&&json[start]=='f'&&json[start+1]=='a'&&json[start+2]=='l'&&json[start+3]=='s'&&json[start+4]=='e'){
        return new_segment(start,start+4);
    }
    return NULL;
}

segment* parse_pair(char* json,int start,int end,char start_char,char end_char){
    if(start>=end||json[start]!=start_char) return NULL;
    int left=1;
    int _end=start;
    do{
        if(left==0){
            return new_segment(start,_end);
        }
        _end++;
        if(json[_end]==start_char) left++;
        else if(json[_end]==end_char) left--;
    }while(_end<end);
    return NULL;
}


segment* parse_obj(char* json,int start,int end){
    return parse_pair(json,start,end,'{','}');
}
segment* parse_arr(char* json,int start,int end){
    return parse_pair(json,start,end,'[',']');
}

int skip_space(char* json,int start,int end){
    while(start<end&&json[start]==' '){
        start++;
    }
    return start;
}


json_obj* parse_json_obj(char* json,int start,int end){
    start=skip_space(json,start,end);
    if(start>=end) return NULL;
    if(json[start]!='{') return NULL;
    start++;
    json_obj* pre_head=new_json_obj(json);
    json_obj* p=pre_head;
    while(start<end){
        start=skip_space(json,start,end);
        key* k=new_key(parse_str(json,start,end));
        if(k==NULL) return NULL;
        start=k->seg->end+1;
        if(start>=end) return NULL;
        start=skip_space(json,start,end);
        if(start>=end||json[start]!=':') return NULL;
        start=skip_space(json,start+1,end);

        //null 0
//number 1
//string 2
//bool 3
//object 4
//array 5
        segment* null=parse_null(json,start,end);
        segment* number=parse_num(json,start,end);
        segment* str=parse_str(json,start,end);
        segment* bool=parse_bool(json,start,end);
        segment* obj=parse_obj(json,start,end);
        segment* arr=parse_arr(json,start,end);
        val* v=NULL;
        if(null!=NULL){
            v=new_val(null,0);
        }else if(number!=NULL){
            v=new_val(number,1);
        }else if(str!=NULL){
            v=new_val(str,2);
        }else if(bool!=NULL){
            v=new_val(bool,3);
        }else if(obj!=NULL){
            v=new_val(obj,4);
        }else if(arr!=NULL){
            v=new_val(arr,5);
        }else{
            return NULL;
        }
        json_obj* t=new_json_obj(json);
        t->item_key=k;
        t->item_val=v;
        t->pre=p;
        p->next=t;
        p=t;
        start=v->seg->end+1;
        start=skip_space(json,start,end);
        if(json[start]=='}') return pre_head->next;
        if(json[start]!=',') return NULL;
        start++;
    }
    return NULL;
}

void print_json_obj(char* tag,json_obj* obj){
    if(obj==NULL){
        printf("%s:%s\n\r",tag,"null");
        return;
    }
    segment* key_seg=obj->item_key->seg;
    segment* val_seg=obj->item_val->seg;
    int type=obj->item_val->type;


    printf("tag:{key:{start:%d,end:%d},val:{start:%d,end:%d,type:%d}}\n\r",key_seg->start,key_seg->end,val_seg->start,val_seg->end,type);


}



int main(){
    char* json="{'Hello':0.111113,'clang':{}}";
    int start=0;
    int end=strlen(json);

    json_obj* obj=parse_json_obj(json,start,end);

    print_json_obj("main.obj",obj->next);

    free_json_obj(obj);
}