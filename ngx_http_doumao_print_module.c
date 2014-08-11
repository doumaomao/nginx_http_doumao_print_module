#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

static char* ngx_doumao_print_readconf(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static void* ngx_doumao_print_create_loc_conf(ngx_conf_t *cf);
static char* ngx_doumao_print_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);

typedef struct {
    ngx_str_t ecdata;
    ngx_flag_t           enable;
} ngx_doumao_print_loc_conf_t;

//从nginx_module_t回调到ngx_doumao_print_commands
static ngx_command_t  ngx_doumao_print_commands[] = {
    { ngx_string("doumao_print"),      //指令的名字，这里的指令是doumao_print。#define ngx_string(str)     { sizeof(str) - 1, (u_char *) str }
      NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,   
	  //确切的说是指令的属性，在什么位置出现，带几个参数.
	  //其中NGX_HTTP_LOC_CONF: 是指指令在location配置部分出现是合法的。
	  //NGX_CONF_TAKE1: 指令读入1个参数 这些参数都定义在nginx的模块指令中，模块的指令是定义在一个叫做ngx_command_t的静态数组中。
      ngx_doumao_print_readconf,      //解析指令的回调函数
      NGX_HTTP_LOC_CONF_OFFSET,                  
      offsetof(ngx_doumao_print_loc_conf_t, ecdata),
      NULL },
      ngx_null_command
};


static ngx_http_module_t  ngx_doumao_print_module_ctx = {
    NULL,                          /* preconfiguration */
    NULL,           /* postconfiguration */

    NULL,                          /* create main configuration */
    NULL,                          /* init main configuration */

    NULL,                          /* create server configuration */
    NULL,                          /* merge server configuration */

    ngx_doumao_print_create_loc_conf,  /* create location configuration */
    ngx_doumao_print_merge_loc_conf /* merge location configuration */
};


//nginx会首先寻找ngx_module_t结构体，该结构体表示一个模块
ngx_module_t  ngx_module_doumao_print = {
    NGX_MODULE_V1,                  //#define NGX_MODULE_V1          0, 0, 0, 0, 0, 0, 1 此处的NGX_MODULE_V1填充ngx_module_t的前面七个字段的值。
    &ngx_doumao_print_module_ctx,  /* module context 以回调函数的形式展现。
    ngx_doumao_print_commands,     /* 模块指令，也是以回调函数形式展现。 该回调函数会在初始化会时被调用，用来解析指令。
    NGX_HTTP_MODULE,               /* 模块类型
    NULL,                          /* init master */
    NULL,                          /* init module */
    NULL,                          /* init process */
    NULL,                          /* init thread */
    NULL,                          /* exit thread */
    NULL,                          /* exit process */
    NULL,                          /* exit master */
    NGX_MODULE_V1_PADDING            //#define NGX_MODULE_V1_PADDING  0, 0, 0, 0, 0, 0, 0, 0 使用该值初始化后面8个数值
};


static ngx_int_t
ngx_doumao_print_handler(ngx_http_request_t *r)
{
     //nginx通过ngx_http_request_t来保存解析请求与输出响应相关的数据。
    printf("called:ngx_doumao_print_handler\n");
    ngx_int_t     rc;
    ngx_buf_t    *b;       //缓冲区
    ngx_chain_t   out;        //单向链表，处理从handler模块传来的数据

    ngx_doumao_print_loc_conf_t  *cglcf;
    cglcf = ngx_http_get_module_loc_conf(r, ngx_module_doumao_print);//获取location的配置

    if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
        return NGX_HTTP_NOT_ALLOWED;
    }
	/*ngx_http_process_request_headers在接收、解析完http请求的头部后，
	会把解析完的每一个http头部加入到headers_in的headers链表中，同时会构造headers_in中的其他成员*/  
	/*if_modified_since标签是http协议中一个IMS标签，主要用于判断是否获取缓存文件*/
    if (r->headers_in.if_modified_since) {
        return NGX_HTTP_NOT_MODIFIED;
    }

    r->headers_out.content_type.len = sizeof("text/html") - 1;
    r->headers_out.content_type.data = (u_char *) "text/html";



    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = cglcf->ecdata.len;

    if (r->method == NGX_HTTP_HEAD) {
        rc = ngx_http_send_header(r);

        if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
            return rc;
        }
    }

    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    out.buf = b;
    out.next = NULL;

    //这块用来将解析的配置的内容，输出到下游的filter中
    b->pos = cglcf->ecdata.data;
    b->last = cglcf->ecdata.data+(cglcf->ecdata.len);

    b->memory = 1;
    b->last_buf = 1;
    rc = ngx_http_send_header(r);

    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }
    return ngx_http_output_filter(r, &out);
}
static char *
ngx_doumao_print_readconf(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    printf("called:ngx_doumao_print_readconf\n");
    ngx_http_core_loc_conf_t  *clcf;

	//ngx_http_conf_get_module_loc_conf是一个宏，用于获得Location相关的配置表cf中ngx_http_core_module对应的项
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module); 
    clcf->handler = ngx_doumao_print_handler;
    ngx_conf_set_str_slot(cf,cmd,conf);
    return NGX_CONF_OK;
}


static void *
ngx_doumao_print_create_loc_conf(ngx_conf_t *cf)
{
    printf("called:ngx_doumao_print_create_loc_conf\n");
    ngx_doumao_print_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_doumao_print_loc_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }

    conf->ecdata.len=0;
    conf->ecdata.data=NULL;
    conf->enable = NGX_CONF_UNSET;
    return conf;
}
static char *
ngx_doumao_print_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    printf("called:ngx_doumao_print_merge_loc_conf\n");
    ngx_doumao_print_loc_conf_t *prev = parent;
    ngx_doumao_print_loc_conf_t *conf = child;

    ngx_conf_merge_str_value(conf->ecdata, prev->ecdata, 10);
    ngx_conf_merge_value(conf->enable, prev->enable, 0);
/**
    if(conf->enable)
        ngx_doumao_print_init(conf);
        */
    return NGX_CONF_OK;
}
