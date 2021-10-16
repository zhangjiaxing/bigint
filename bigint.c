#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef struct bigint{
    uint8_t data[16];  //0下标存放最低位
} bigint_t;

void bigint_add_uint(const bigint_t *a, uint8_t n, bigint_t *ret){
    uint8_t flag = 0;
    uint8_t tmp = 0; //用于保证当ret 和 a相等时, 不出错
    for(int i=0; i<16; i++){
        tmp = a->data[i] + n + flag;
        flag = tmp < a->data[i] | tmp < n;
        n = 0;
        ret->data[i] = tmp;
    }
}

void bigint_add(const bigint_t *a, const bigint_t *b, bigint_t *ret){
    uint8_t flag = 0;
    uint8_t tmp = 0;
    for(int i=0; i<16; i++){
        tmp = a->data[i] + b->data[i] + flag;
        flag = tmp < a->data[i] | tmp < b->data[i];
        ret->data[i] = tmp;
    }
}

void bigint_sub(const bigint_t *a, const bigint_t *b, bigint_t *ret){
    bigint_t tmp;
    for(int i=0; i<16; i++){
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

void bigint_bit_shift_left(bigint_t *a, unsigned n){ //n最大不能超过8, 否则出问题
    int8_t overflow = 0;
    int8_t prev = 0;

    for(int i=0; i<16; i++){
        overflow = a->data[i] >> (8-n);
        a->data[i] = a->data[i] << n;
        a->data[i] |= prev;
        prev = overflow;
    }
}

void bigint_mul(const bigint_t *a, const bigint_t  *b, bigint_t *ret){
    memset(ret, 0, sizeof(ret));
    bigint_t tmp = *a;
    for(uint8_t i=0; i<16; i++){
        uint8_t bytes = b->data[i];
        while(bytes != 0){
            if(bytes & 0x1){
                bigint_add((const bigint_t *)ret, &tmp, ret);
            }
            bigint_bit_shift_left(&tmp, 1);
            bytes = bytes >> 1;
        }
    }
}

void bigint_div(const bigint_t *a, const bigint_t  *b, uint8_t *quotient, uint8_t *remainder){
    uint8_t count = -1;
    bigint_t tmp = *b;
    int8_t cmp = bigint_cmp(a, &tmp);
    if(cmp < 0){
        *quotient = 0;
        *remainder = a->data[0];
        return;
    }
    while(bigint_cmp(a, &tmp) >= 0){
        count++; //a 大于等于 b * 2 ^ count. 或者说 a 大于等于 b << count
        bigint_bit_shift_left(&tmp, 1);
    }
    tmp = *b;
    //FIXME
    bigint_bit_shift_left(&tmp, count);

    bigint_sub(a, &tmp, &tmp);
    uint8_t tmp_quot, tmp_remain;
    bigint_div(&tmp, b, &tmp_quot, &tmp_remain);
    
    *quotient = (1<<count) + tmp_quot;
    *remainder = tmp_remain;
}

void print_mem(uint8_t *addr, uint8_t len){
    while(len--){
        printf("%02X ", *addr);
        addr++;
    }
    printf("\n");
}


void test_add(){
    bigint_t d1 = {};
    bigint_t d2 = {};
    bigint_add_uint(&d1, 128, &d1);
    bigint_add_uint(&d2, 127, &d2);
    
    bigint_t ret;
    bigint_add(&d1, &d2, &ret);
    print_mem((void *) &ret, 16);

    bigint_sub(&ret, &d1, &d1);
    print_mem((void *) &d1, 16);
}

void test_mul(){
    bigint_t n1 = {};
    bigint_add_uint(&n1, 255, &n1);
    print_mem((void *) &n1, 16);

    bigint_t n2 = {};
    bigint_add_uint(&n2, 255, &n2);

    bigint_t ret;

    bigint_mul(&n1, &n1, &ret);
    bigint_mul(&ret, &n1, &n2);
    bigint_mul(&n2, &n1, &ret);
    bigint_mul(&ret, &n1, &n2);
    bigint_mul(&n2, &n1, &ret);

    print_mem((void *) &ret, 16);
}

int main(){
    bigint_t n1 = {};
    bigint_add_uint(&n1, 27, &n1);

    bigint_t n2 = {};
    bigint_add_uint(&n2, 5, &n2);

    uint8_t quot, remain;
    bigint_div(&n1, &n2, &quot, &remain);
    printf("%d %d\n", quot, remain);

    return 0;
}

