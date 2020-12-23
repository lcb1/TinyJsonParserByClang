# TinyJsonParserByClang
### 简介  
> 一个极简的C语言json解析器  
> 一次扫描解析 只记录key与value在原串的区间 还有value的type

* json_obj
```
typedef struct json_obj{
    struct json_obj* pre; //上一个json_obj
    struct json_obj* next; //下一个json_obj
    char* json;             //原json串
    key* item_key;        //key 在原串的区间
    val* item_val;      //val 在原串的区间
}json_obj;
```
* key
```
typedef struct key{
    int start;
    int end;
}key;
```

* val
```
typedef struct{
    int start;
    int end;
    int type;
}val; // 0:null 1:number 2:bool 3:string
```
