// -------------------------------------------------------------------------------------
// this is the only file you need to edit
// -------------------------------------------------------------------------------------
//
// (c) 2021, Pavol Federl, pfederl@ucalgary.ca
// Do not distribute this file.

#include "memsim.h"
#include <cassert>
#include <iostream>
#include <list>
#include <set>
#include <unordered_map>

struct Partition {
    int tag;
    int64_t size;
    int64_t addr;
};
typedef std::list<Partition>::iterator PartitionRef;

struct scmp {
    bool operator()(const PartitionRef& c1, const PartitionRef& c2) const {
        if (c1->size == c2->size)
            return c1->addr < c2->addr;
        else
            return c1->size > c2->size;
    }
};

// I recommend you implement the simulator as a class. This is only a suggestion.
// If you decide not to use this class, please remove it before submitting it.
struct Simulator {
    int64_t page_size;
    // all partitions, in a linked list
    std::list<Partition> all_blocks;
    // sorted free partitions by size/address
    std::set<PartitionRef, scmp> free_blocks;
    // quick access to all tagged partitions
    std::unordered_map<long, std::vector<PartitionRef>> tagged_blocks;

    MemSimResult result;

    Simulator(int64_t size_of_page)
    {
        result.n_pages_requested = 0;
        // constructor
        page_size = size_of_page;
    }
    void allocate(int tag, int size)
    {
        if(!all_blocks.empty()){
            if(!free_blocks.empty()){
                PartitionRef temp(*free_blocks.begin());
                if(temp->size > size){
                    free_blocks.erase(free_blocks.begin());
                    int64_t leftover = (temp->size) - size;
                    temp->tag = tag;
                    temp->size = size;
                    temp = all_blocks.insert(std::next(temp), (Partition){-1, leftover, (temp->addr)+(temp->size)});
                    tagged_blocks[tag].push_back(PartitionRef(std::prev(temp)));
                    free_blocks.insert(PartitionRef(temp));
                }
                else if(temp->size == size){
                    free_blocks.erase(free_blocks.begin());
                    temp->tag = tag;
                    tagged_blocks[tag].push_back(PartitionRef(temp));
                }
                else{
                    int64_t n_pages;
                    if(all_blocks.back().tag != -1){
                        if(size%page_size==0){
                            n_pages = size/page_size;
                            result.n_pages_requested += n_pages;
                            int64_t naddr = all_blocks.back().addr + all_blocks.back().size;
                            all_blocks.push_back((Partition){tag, size, naddr});
                            tagged_blocks[tag].push_back(PartitionRef(std::prev(all_blocks.end())));
                        }
                        else{
                            n_pages = (size/page_size)+1;
                            result.n_pages_requested += n_pages;
                            int64_t naddr = all_blocks.back().addr + all_blocks.back().size;
                            all_blocks.push_back((Partition){tag, size, naddr});
                            tagged_blocks[tag].push_back(PartitionRef(std::prev(all_blocks.end())));
                            naddr = all_blocks.back().addr + all_blocks.back().size;
                            all_blocks.push_back((Partition){-1, (page_size*n_pages)-size, naddr});
                            free_blocks.insert(PartitionRef(std::prev(all_blocks.end())));
                        }
                    }
                    else{
                        if((size-(all_blocks.back().size))%page_size==0){
                            n_pages = (size-(all_blocks.back().size))/page_size;
                            result.n_pages_requested += n_pages;
                            free_blocks.erase(free_blocks.find(std::prev(all_blocks.end())));
                            all_blocks.back().tag = tag;
                            all_blocks.back().size = size;
                            tagged_blocks[tag].push_back(PartitionRef(std::prev(all_blocks.end())));
                        }
                        else{
                            n_pages = ((size-(all_blocks.back().size))/page_size)+1;
                            result.n_pages_requested += n_pages;
                            //std::cout << "\nreqested pages = "<<n_pages<<std::endl;
                            free_blocks.erase(free_blocks.find(std::prev(all_blocks.end())));
                            int64_t leftover = (page_size*n_pages)-(size-(all_blocks.back().size));
                            all_blocks.back().tag = tag;
                            all_blocks.back().size = size;
                            tagged_blocks[tag].push_back(PartitionRef(std::prev(all_blocks.end())));

                            int64_t naddr = all_blocks.back().addr + all_blocks.back().size;
                            all_blocks.push_back((Partition){-1, leftover, naddr});
                            free_blocks.insert(PartitionRef(std::prev(all_blocks.end())));
                        }
                    }
                }
            }
            else{
                int64_t n_pages;
                if(all_blocks.back().tag != -1){
                    if(size%page_size==0){
                        n_pages = size/page_size;
                        result.n_pages_requested += n_pages;
                        int64_t naddr = all_blocks.back().addr + all_blocks.back().size;
                        all_blocks.push_back((Partition){tag, size, naddr});
                        tagged_blocks[tag].push_back(PartitionRef(std::prev(all_blocks.end())));
                    }
                    else{
                        n_pages = (size/page_size)+1;
                        result.n_pages_requested += n_pages;
                        int64_t naddr = all_blocks.back().addr + all_blocks.back().size;
                        all_blocks.push_back((Partition){tag, size, naddr});
                        tagged_blocks[tag].push_back(PartitionRef(std::prev(all_blocks.end())));
                        naddr = all_blocks.back().addr + all_blocks.back().size;
                        all_blocks.push_back((Partition){-1, (page_size*n_pages)-size, naddr});
                        free_blocks.insert(PartitionRef(std::prev(all_blocks.end())));
                    }
                }
                else{
                    if((size-(all_blocks.back().size))%page_size==0){
                        n_pages = (size-(all_blocks.back().size))/page_size;
                        result.n_pages_requested += n_pages;
                        free_blocks.erase(free_blocks.find(std::prev(all_blocks.end())));
                        all_blocks.back().tag = tag;
                        all_blocks.back().size = size;
                        tagged_blocks[tag].push_back(PartitionRef(std::prev(all_blocks.end())));
                    }
                    else{
                        n_pages = ((size-(all_blocks.back().size))/page_size)+1;
                        result.n_pages_requested += n_pages;
                        free_blocks.erase(free_blocks.find(std::prev(all_blocks.end())));
                        all_blocks.back().tag = tag;
                        all_blocks.back().size = size;
                        tagged_blocks[tag].push_back(PartitionRef(std::prev(all_blocks.end())));
                        int64_t leftover = (page_size*n_pages)-(size-(all_blocks.back().size));
                        int64_t naddr = all_blocks.back().addr + all_blocks.back().size;
                        all_blocks.push_back((Partition){-1, leftover, naddr});
                        free_blocks.insert(PartitionRef(std::prev(all_blocks.end())));
                    }
                }
            }
        }
        else{
            int64_t n_pages;
            if(size%page_size==0){
                n_pages = size/page_size;
                result.n_pages_requested += n_pages;
                all_blocks.push_back((Partition){tag,page_size*n_pages,0});
                tagged_blocks[tag].push_back(PartitionRef(all_blocks.begin()));
            }
            else{
                n_pages = (size/page_size)+1;
                result.n_pages_requested += n_pages;
                all_blocks.push_back((Partition){tag,size,0});
                all_blocks.push_back((Partition){-1,(page_size*n_pages)-size,size});
                tagged_blocks[tag].push_back(PartitionRef(all_blocks.begin()));
                free_blocks.insert(PartitionRef(std::next(all_blocks.begin())));
            }
        }
        // Pseudocode for allocation request:
        // - search through the list of partitions from start to end, and
        //   find the largest partition that fits requested size
        //     - in case of ties, pick the first partition found
        // - if no suitable partition found:
        //     - get minimum number of pages from OS, but consider the
        //       case when last partition is free
        //     - add the new memory at the end of partition list
        //     - the last partition will be the best partition
        // - split the best partition in two if necessary
        //     - mark the first partition occupied, and store the tag in it
        //     - mark the second partition free
    }
    void deallocate(int tag)
    {
        if(tagged_blocks.find(tag) != tagged_blocks.end()){
            for(PartitionRef pr : tagged_blocks[tag]){
                //std::cout<<"address: "<<pr->addr<<std::endl;
                if(pr->addr == 0 && pr->addr+pr->size != result.n_pages_requested*page_size){
                    //std::cout<<"eli hehe"<<std::endl;
                    if(std::next(pr)->tag != -1){
                        pr->tag = -1;
                        free_blocks.insert(PartitionRef(pr));
                    }
                    else{
                        free_blocks.erase(free_blocks.find(std::next(pr)));
                        pr->tag = -1;
                        pr->size += std::next(pr)->size;
                        free_blocks.insert(PartitionRef(pr));
                        all_blocks.erase(std::next(pr));
                    }
                }
                else if(pr->addr != 0 && pr->addr+pr->size == result.n_pages_requested*page_size){
                    if(std::prev(pr)->tag != -1){
                        pr->tag = -1;
                        free_blocks.insert(PartitionRef(pr));
                    }
                    else{
                        free_blocks.erase(free_blocks.find(std::prev(pr)));
                        std::prev(pr)->size += pr->size;
                        free_blocks.insert(PartitionRef(std::prev(pr)));
                        all_blocks.erase(pr);
                    }
                }
                else if(pr->addr == 0 && pr->addr+pr->size == result.n_pages_requested*page_size){
                    pr->tag = -1;
                    free_blocks.insert(PartitionRef(pr));
                }
                else{
                    //std::cout << "sarah hsdhdhhds"<<std::endl;
                    if(std::next(pr)->tag != -1 && std::prev(pr)->tag != -1){
                        pr->tag = -1;
                        free_blocks.insert(PartitionRef(pr));
                    }
                    else if(std::next(pr)->tag != -1 && std::prev(pr)->tag == -1){
                        free_blocks.erase(free_blocks.find(std::prev(pr)));
                        std::prev(pr)->size += pr->size;
                        free_blocks.insert(PartitionRef(std::prev(pr)));
                        all_blocks.erase(pr);
                    }
                    else if(std::next(pr)->tag == -1 && std::prev(pr)->tag != -1){
                        free_blocks.erase(free_blocks.find(std::next(pr)));
                        pr->tag = -1;
                        pr->size += std::next(pr)->size;
                        free_blocks.insert(PartitionRef(pr));
                        all_blocks.erase(std::next(pr));
                    }
                    else if(std::next(pr)->tag == -1 && std::prev(pr)->tag == -1){
                        free_blocks.erase(free_blocks.find(std::next(pr)));
                        free_blocks.erase(free_blocks.find(std::prev(pr)));
                        std::prev(pr)->size += pr->size;
                        std::prev(pr)->size += std::next(pr)->size;
                        free_blocks.insert(PartitionRef(std::prev(pr)));
                        pr = all_blocks.erase(pr);
                        all_blocks.erase(pr);
                    }
                }
            }
            //std::cout<<"clearing"<< std::endl;
            tagged_blocks.erase(tagged_blocks.find(tag));
        }
        // Pseudocode for deallocation request:
        // - for every partition
        //     - if partition is occupied and has a matching tag:
        //         - mark the partition free
        //         - merge any adjacent free partitions
    }
    MemSimResult getStats()
    {
        // let's guess the result... :)
        if(free_blocks.empty()){
            result.max_free_partition_size = 0;
            result.max_free_partition_address = 0;
        }
        else{
            PartitionRef temp(*free_blocks.begin());
            result.max_free_partition_size = temp->size;
            result.max_free_partition_address = temp->addr;
        }
        return result;
    }
    void check_consistency()
    {
        std::cout<<"\nchecking to see if the sum of all partition sizes in the linked list is the same as number of page requests * page_size"<<std::endl;
        int64_t  sum = 0;
        std::cout<<"\nAll blocks"<<std::endl;
        for(Partition p : all_blocks){
            std::cout<<"tag: "<<p.tag<<"  address: "<<p.addr<<"  size: "<<p.size<<"\n";
            sum += p.size;
        }
        std::cout<<"\nTagged blocks"<<std::endl;
        for(auto const &pair : tagged_blocks){
            std::cout<<"tag: "<<pair.first<<"  :";
            for(PartitionRef b : pair.second){
                std::cout<<" "<<b->addr;
            }
            std::cout<<"}\n";
        }
        std::cout<<"\nsum of all partitions = "<<sum<<std::endl;
        std::cout<<"page requests * page_size = "<<result.n_pages_requested*page_size<<std::endl;


        // you do not need to implement this method at all - this is just my suggestion
        // to help you with debugging

        // mem_sim() calls this after every request to make sure all data structures
        // are consistent. Since this will probablly slow down your code, you should
        // disable calling this in the mem_sim() function below before submitting
        // your code for grading.

        // here are some suggestions for consistency checks (see appendix also):

        // make sure the sum of all partition sizes in your linked list is
        // the same as number of page requests * page_size

        // make sure your addresses are correct

        // make sure the number of all partitions in your tag data structure +
        // number of partitions in your free blocks is the same as the size
        // of the linked list

        // make sure that every free partition is in free blocks

        // make sure that every partition in free_blocks is actually free

        // make sure that none of the partition sizes or addresses are < 1
    }
};

// re-implement the following function
// ===================================
// parameters:
//    page_size: integer in range [1..1,000,000]
//    requests: array of requests
// return:
//    some statistics at the end of simulation
MemSimResult mem_sim(int64_t page_size, const std::vector<Request> & requests)
{
    // if you decide to use the simulator class, you probably do not need to make
    // any changes to the code below, except to uncomment the call to check_consistency()
    // before submitting your code
    Simulator sim(page_size);

    for (const auto & req : requests) {
        //std::cout<<"tag: "<<req.tag<<"    size: "<<req.size<<std::endl;
        if (req.tag < 0) {
            sim.deallocate(-req.tag);
        } else {
            sim.allocate(req.tag, req.size);
        }
        //sim.check_consistency();
    }
    return sim.getStats();
}
