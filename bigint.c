#include <stdio.h>
#include <string.h>
#include <stdint.h>
//假设C语言仅支持占用空间8bit的数字, 实现大数加减乘除

typedef struct bigint{
    uint8_t data[16];  //0下标存放最低位
} bigint_t;

void bigint_add_uint(const bigint_t *a, uint8_t n, bigint_t *ret){
    uint8_t flag = 0;
    uint8_t tmp = 0; //用于保证当ret 和 a相等时, 不出错
    for(uint8_t i=0; i<16; i++){
        tmp = a->data[i] + n + flag;
        flag = tmp < a->data[i] | tmp < n;
        n = 0;
        ret->data[i] = tmp;
    }
}

void bigint_add(const bigint_t *a, const bigint_t *b, bigint_t *ret){
    uint8_t flag = 0;
    uint8_t tmp = 0;
    for(uint8_t i=0; i<16; i++){
        tmp = a->data[i] + b->data[i] + flag;
        flag = tmp < a->data[i] | tmp < b->data[i];
        ret->data[i] = tmp;
    }
}

void bigint_sub(const bigint_t *a, const bigint_t *b, bigint_t *ret){
    bigint_t tmp;
    for(uint8_t i=0; i<16; i++){
        tmp.data[i] = ~(b->data[i]);
    }
    bigint_add_uint(&tmp, 1, &tmp);
    bigint_add(a, &tmp, ret);
}

int8_t bigint_cmp(const bigint_t *a, const bigint_t  *b){
    int8_t i = 15;
    while(i > 0 && a->data[i] == b->data[i]){
        i--;
    }
    //return a->data[i] - b->data[i]; //溢出
    if (a->data[i] < b->data[i]){
        return -1;
    }else if(a->data[i] == b->data[i]){
        return 0;
    }else{
        return 1;
    }
}

void bigint_bit_shift_left(bigint_t *a, uint8_t n){ //n最大不能超过8, 否则出问题
    while(n > 8){
        n -= 8;
        bigint_bit_shift_left(a, 8);
    }

    int8_t overflow = 0;
    int8_t prev = 0;

    for(uint8_t i=0; i<16; i++){
        overflow = a->data[i] >> (8-n);
        a->data[i] = a->data[i] << n;
        a->data[i] |= prev;
        prev = overflow;
    }
}

void bigint_mul(const bigint_t *a, const bigint_t  *b, bigint_t *ret){
    bigint_t tmp_ret = {}; //为了在ret和a或者b相等时,不出错, 不能直接修改ret
    bigint_t tmp = *a;
    for(uint8_t i=0; i<16; i++){
        uint8_t bytes = b->data[i];
        while(bytes != 0){
            if(bytes & 0x1){
                bigint_add(&tmp_ret, &tmp, &tmp_ret);
            }
            bigint_bit_shift_left(&tmp, 1);
            bytes = bytes >> 1;
        }
    }
    *ret = tmp_ret;
}

void bigint_div(const bigint_t *a, const bigint_t  *b, bigint_t *quotient, bigint_t *remainder){
    uint8_t count = -1;
    bigint_t tmp = *b;
    int8_t cmp = bigint_cmp(a, &tmp);
    if(cmp < 0){
        memset(quotient, 0, sizeof(*quotient));  // *quotient = 0;
        *remainder = *a;
        return;
    }
    while(bigint_cmp(a, &tmp) >= 0){
        count++; //a 大于等于 b * 2 ^ count. 或者说 a 大于等于 b << count
        bigint_bit_shift_left(&tmp, 1);
    }
    tmp = *b;
    bigint_bit_shift_left(&tmp, count);

    bigint_sub(a, &tmp, &tmp);
    bigint_t tmp_quot = {}, tmp_remain = {};
    bigint_div(&tmp, b, &tmp_quot, &tmp_remain);
    
    //*quotient = (1<<count) + tmp_quot;  //1<<count可能会溢出
    bigint_t i1 = {};
    bigint_add_uint(&i1, 1, &i1);
    bigint_bit_shift_left(&i1, count);
    bigint_add(&i1, &tmp_quot, &tmp_quot);

    *quotient = tmp_quot;
    *remainder = tmp_remain;
}

void bigint_atoi(const char *str, bigint_t *bi){ //字符串转换为大数
    memset(bi, 0, sizeof(*bi));

    bigint_t i10 = {};
    bigint_add_uint(&i10, 10, &i10);

    while(*str != '\0'){
        bigint_mul(bi, &i10, bi);
        bigint_add_uint(bi, *str-'0', bi);
        str++;
    }
}

void print_mem(uint8_t *addr, uint8_t len){
    while(len--){
        printf("%02X ", *addr);
        addr++;
    }
    printf("\n");
}


void test_add(){
    printf("add: \n");
    bigint_t d1 = {};
    bigint_t d2 = {};
    bigint_atoi("123456789", &d1);
    bigint_atoi("123456789", &d2);

    bigint_t ret;
    bigint_add(&d1, &d2, &ret);
    print_mem((void *) &ret, 16);
}

void test_sub(){
    printf("sub: \n");
    bigint_t d1 = {};
    bigint_t d2 = {};
    bigint_atoi("123456789", &d1);
    bigint_atoi("1234", &d2);
    
    bigint_t ret;
    bigint_sub(&d1, &d2, &ret);
    print_mem((void *) &ret, 16);
}

void test_mul(){
    printf("mul: \n");
    bigint_t n1 = {};
    bigint_atoi("123", &n1);

    bigint_t n2 = {};
    bigint_atoi("123456789", &n2); //FIXME

    bigint_t ret = {};
    bigint_mul(&n1, &n1, &ret);
    //bigint_mul(&ret, &n1, &ret);
    print_mem((void *) &ret, 16);
}

void test_div(){
    printf("div: \n");
    bigint_t n1 = {};
    bigint_atoi("123456789", &n1);

    bigint_t n2 = {};
    bigint_atoi("1234567", &n2);

    bigint_t quot = {}, remain = {};
    bigint_div(&n1, &n2, &quot, &remain);
    print_mem((void *) &quot, 16);
    print_mem((void *) &remain, 16);
    // printf("%d %d\n", quot, remain);
}

void test_atoi(){
    printf("atoi: \n");
    const char *s = "123456789123456789";
    bigint_t i = {};
    bigint_atoi(s, &i);
    print_mem((void *) &i, 16);
}

int main(){
    test_add();
    test_sub();
    test_mul();
    test_div();
    test_atoi();

    return 0;
}

