#ifndef _BITSET_INTERSECTION_H_
#define _BITSET_INTERSECTION_H_

namespace ops{
  template<typename F, typename FA, typename FB, typename FCA, typename FCB, typename FEA, typename FEB>
  inline void find_matching_offsets(const uint8_t *A, 
    const size_t lenA,
    const size_t increment_a, 
    FA fa,
    FCA fca,
    FEA fea,
    const uint8_t *B,
    const size_t lenB, 
    const size_t increment_b,
    FB fb,
    FCB fcb,
    FEB feb, 
    F f){

      uint32_t a_index = 0;
      uint32_t b_index = 0;
      const uint8_t *endA = A + lenA*increment_a;
      const uint8_t *endB = B + lenB*increment_b;

      if (lenA == 0){
        feb(B,endB,increment_b);
        return;
      } else if(lenB == 0){
        fea(A,endA,increment_a);
        return;
      }

      while (1) {
          while (fa(*((uint32_t*)A)) < fb(*((uint32_t*)B))) {
  SKIP_FIRST_COMPARE:
              fca(*(uint32_t*)A);
              A += increment_a;
              ++a_index;
              if (A == endA){
                feb(B,endB,increment_b);
                return;
              }
          }
          while (fa(*((uint32_t*)A)) > fb(*((uint32_t*)B)) ) {
              fcb(*(uint32_t*)B);
              B += increment_b;
              ++b_index;
              if (B == endB){
                fea(A,endA,increment_a);
                return;
              }
          }
          if (fa(*((uint32_t*)A)) == fb(*((uint32_t*)B))) {
              auto pair = f(a_index,b_index,*(uint32_t*)A);
              A += increment_a*std::get<0>(pair);
              a_index += std::get<0>(pair);
              B += increment_b*std::get<1>(pair);
              b_index += std::get<1>(pair);
              if (A == endA){
                feb(B,endB,increment_b);
                return;
              } else if (B == endB){
                fea(A,endA,increment_a);
                return;
              }
          } else {
              goto SKIP_FIRST_COMPARE;
          }
      }
      return; // NOTREACHED
  }

  inline size_t intersect_range_block(
    uint64_t * const result_data, 
    uint32_t * const index_data, 
    const uint64_t * const A, 
    const uint64_t * const B,
    const size_t b_size){
    
    size_t i = 0;
    size_t count = 0;

    #if VECTORIZE == 1
    while((i+255) < b_size){
      const size_t vector_index = (i/64);
      const __m256 m1 = _mm256_loadu_ps((float*)(A + vector_index));
      const __m256 m2 = _mm256_loadu_ps((float*)(B + vector_index));
      const __m256 r = _mm256_and_ps(m1, m2);
      
      _mm256_storeu_ps((float*)(result_data+vector_index), r);
      
      index_data[vector_index] = count;
      count += _mm_popcnt_u64(result_data[vector_index]);
      index_data[vector_index+1] = count;
      count += _mm_popcnt_u64(result_data[vector_index+1]);
      index_data[vector_index+2] = count;
      count += _mm_popcnt_u64(result_data[vector_index+2]);
      index_data[vector_index+3] = count;
      count += _mm_popcnt_u64(result_data[vector_index+3]);
      
      i += 256;
    }
    #endif

    //64 bits per word
    for(; i < b_size; i+=64){
      const size_t vector_index = (i/64);
      const uint64_t r = A[vector_index] & B[vector_index]; 
      result_data[vector_index] = r;
      index_data[vector_index] = count;
      count += _mm_popcnt_u64(r);
    }

    return count;
  }

  inline size_t intersect_block(
    uint64_t * const result_data, 
    const uint64_t * const A, 
    const uint64_t * const B,
    const size_t b_size){
    
    size_t i = 0;
    size_t count = 0;

    #if VECTORIZE == 1
    while((i+255) < b_size){
      const size_t vector_index = (i/64);
      const __m256 m1 = _mm256_loadu_ps((float*)(A + vector_index));
      const __m256 m2 = _mm256_loadu_ps((float*)(B + vector_index));
      const __m256 r = _mm256_and_ps(m1, m2);
      
      _mm256_storeu_ps((float*)(result_data+vector_index), r);
      
      count += _mm_popcnt_u64(result_data[vector_index]);
      count += _mm_popcnt_u64(result_data[vector_index+1]);
      count += _mm_popcnt_u64(result_data[vector_index+2]);
      count += _mm_popcnt_u64(result_data[vector_index+3]);
      
      i += 256;
    }
    #endif

    //64 bits per word
    for(; i < b_size; i+=64){
      const size_t vector_index = (i/64);
      const uint64_t r = A[vector_index] & B[vector_index]; 
      result_data[vector_index] = r;
      count += _mm_popcnt_u64(r);
    }

    return count;
  }

  inline Set<range_bitset>* set_intersect(Set<range_bitset> *C_in, const Set<range_bitset> *A_in, const Set<range_bitset> *B_in){
    long count = 0l;
    C_in->number_of_bytes = 0;

    if(A_in->number_of_bytes > 0 && B_in->number_of_bytes > 0){
      const uint64_t *a_index = (uint64_t*) A_in->data;
      const uint64_t *b_index = (uint64_t*) B_in->data;

      uint64_t * const C = (uint64_t*)(C_in->data+sizeof(uint64_t));
      const uint64_t * const A = (uint64_t*)(A_in->data+sizeof(uint64_t));
      const uint64_t * const B = (uint64_t*)(B_in->data+sizeof(uint64_t));
      const size_t s_a = ((A_in->number_of_bytes-sizeof(uint64_t))/(sizeof(uint64_t)+sizeof(uint32_t)));
      const size_t s_b = ((B_in->number_of_bytes-sizeof(uint64_t))/(sizeof(uint64_t)+sizeof(uint32_t)));

      const bool a_big = a_index[0] > b_index[0];
      const uint64_t start_index = (a_big) ? a_index[0] : b_index[0];
      const uint64_t a_start_index = (a_big) ? 0:(b_index[0]-a_index[0]);
      const uint64_t b_start_index = (a_big) ? (a_index[0]-b_index[0]):0;

      const uint64_t end_index = ((a_index[0]+s_a) > (b_index[0]+s_b)) ? (b_index[0]+s_b):(a_index[0]+s_a);
      const uint64_t total_size = (start_index > end_index) ? 0:(end_index-start_index);

      //16 uint16_ts
      //8 ints
      //4 longs

      uint32_t * const index_write = (uint32_t*)(total_size+C);
      uint64_t * const c_index = (uint64_t*) C_in->data;
      c_index[0] = start_index;
    
      count = intersect_range_block(C,index_write,A+a_start_index,B+b_start_index,total_size*64);

      C_in->number_of_bytes = total_size*(sizeof(uint64_t)+sizeof(uint32_t))+sizeof(uint64_t);
    }

    const double density = 0.0;
    C_in->cardinality = count;
    C_in->density = density;
    C_in->type= type::RANGE_BITSET;

    return C_in;
  }
  inline Set<block_bitset>* set_intersect(Set<block_bitset> *C_in, const Set<block_bitset> *A_in, const Set<range_bitset> *B_in){
    size_t count = 0;
    size_t num_bytes = 0;

    if(A_in->number_of_bytes > 0 && B_in->number_of_bytes > 0){
      const size_t A_num_blocks = A_in->number_of_bytes/(2*sizeof(uint32_t)+(BLOCK_SIZE/8));
      uint8_t *C = C_in->data;
      const uint32_t offset = 2*sizeof(uint32_t)+WORDS_PER_BLOCK*sizeof(uint64_t);

      const uint64_t *b_index = (uint64_t*) B_in->data;
      const uint64_t * const B = (uint64_t*)(B_in->data+sizeof(uint64_t));
      const size_t s_b = ((B_in->number_of_bytes-sizeof(uint64_t))/(sizeof(uint64_t)+sizeof(uint32_t)));
      const uint64_t b_end = (b_index[0]+s_b);
      const uint64_t b_start = b_index[0];

      for(size_t i = 0; i < A_num_blocks; i++){
        uint32_t index = WORDS_PER_BLOCK * (*((uint32_t*)(A_in->data+i*offset)));
        if( (index+WORDS_PER_BLOCK-1) >= b_start && index < b_end){
          size_t j = 0;
          uint64_t *A_data = (uint64_t*)(A_in->data+i*offset+2*sizeof(uint32_t));
          *((uint32_t*)C) = index/WORDS_PER_BLOCK;
          *((uint32_t*)(C+sizeof(uint32_t))) = count;
          const size_t old_count = count;
          while(index < b_start){
            *((uint64_t*)(C+2*sizeof(uint32_t)+j*sizeof(uint64_t))) = 0;
            index++;
            j++;
          }
          while(j < WORDS_PER_BLOCK && index < b_end){
            uint64_t result = A_data[j] & B[index-b_start];
            *((uint64_t*)(C+2*sizeof(uint32_t)+j*sizeof(uint64_t))) = result; 
            count += _mm_popcnt_u64(result);
            j++;
            index++;
          }
          while(j < WORDS_PER_BLOCK){
            *((uint64_t*)(C+2*sizeof(uint32_t)+j*sizeof(uint64_t))) = 0;
            j++;
          }
          if(old_count != count){
            num_bytes += offset;
            C += offset;
          }
        }
        if(index >= b_end)
          break;
      }
    }

    const double density = 0.0;
    C_in->cardinality = count;
    C_in->number_of_bytes = num_bytes;
    C_in->density = density;
    C_in->type= type::BLOCK_BITSET;

    return C_in;
  }
  inline Set<block_bitset>* set_intersect(Set<block_bitset> *C_in, const Set<range_bitset> *A_in, const Set<block_bitset> *B_in){
    return set_intersect(C_in,B_in,A_in);
  }
  inline Set<block_bitset>* set_intersect(Set<block_bitset> *C_in,const Set<block_bitset> *A_in,const Set<block_bitset> *B_in){
    if(A_in->number_of_bytes == 0 || B_in->number_of_bytes == 0){
      C_in->cardinality = 0;
      C_in->number_of_bytes = 0;
      C_in->density = 0.0;
      C_in->type= type::BLOCK_BITSET;
      return C_in;
    }

    const size_t A_num_blocks = A_in->number_of_bytes/(2*sizeof(uint32_t)+(BLOCK_SIZE/8));
    const size_t B_num_blocks = B_in->number_of_bytes/(2*sizeof(uint32_t)+(BLOCK_SIZE/8));

    size_t count = 0;
    size_t num_bytes = 0;
    const size_t bytes_per_block = (BLOCK_SIZE/8);

    uint8_t *C = C_in->data;
    const uint8_t * const A_data = A_in->data+2*sizeof(uint32_t);
    const uint8_t * const B_data = B_in->data+2*sizeof(uint32_t);
    const uint32_t offset = 2*sizeof(uint32_t)+WORDS_PER_BLOCK*sizeof(uint64_t);
    
    auto check_f = [&](uint32_t d){(void) d; return;};
    auto finish_f = [&](const uint8_t *start, const uint8_t *end, size_t increment){
      (void) start, (void) end; (void) increment;
      return;};

    find_matching_offsets(A_in->data,A_num_blocks,offset,[&](uint32_t a){return a;},check_f,finish_f,
        B_in->data,B_num_blocks,offset,[&](uint32_t b){return b;},check_f,finish_f, 
        
        [&](uint32_t a_index, uint32_t b_index, uint32_t data){    
          *((uint32_t*)C) = data;
          *((uint32_t*)(C+sizeof(uint32_t))) = count; 
          const size_t old_count = count;
          count += intersect_block((uint64_t*)(C+2*sizeof(uint32_t)),(uint64_t*)(A_data+a_index*offset),(uint64_t*)(B_data+b_index*offset),BLOCK_SIZE);
          if(old_count != count){
            C += 2*sizeof(uint32_t)+bytes_per_block;
            num_bytes += 2*sizeof(uint32_t)+bytes_per_block;
          }
          return std::make_pair(1,1); 
        }
    );   

    C_in->cardinality = count;
    C_in->number_of_bytes = num_bytes;
    C_in->density = 0.0;
    C_in->type= type::BLOCK_BITSET;

    return C_in;
  }
}
#endif
