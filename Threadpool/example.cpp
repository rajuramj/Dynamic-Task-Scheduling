#include <iostream>
#include <vector>
#include <chrono>

#include "ThreadPool.h"

int getDouble(int arg)
{
    std::cout << "I am in getDouble() function" << std::endl;
    return 2*arg;
}

int main()
{
    ThreadPool pool(2);
    std::vector< std::future<int> > results;
    //int i=1;

     // Call a lambda function
     results.emplace_back(
                 pool.enqueue ([](int arg) { return 7*arg;}, 11)
                 );

//     // Call a function defined normally
//     results.emplace_back(
//                 pool.enqueue ( getDouble(int), 11)
//                 );


    //for(int i = 0; i < 8; ++i) {
        results.emplace_back(
            pool.enqueue([] {
                //std::cout << "hello " << i << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                //std::cout << "world " << i << std::endl;
                return 7;
            })
        );
    //}

    for(auto && result: results)
        std::cout << result.get() << ' ';
    std::cout << std::endl;
    
    return 0;
}
