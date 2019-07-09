#include <iostream>
#include <boost/interprocess/managed_shared_memory.hpp>
#include "cls.hpp"

using namespace std;
using namespace boost::interprocess;
using namespace NN::MM;

#define SHM_NAME "test_shm"
#define SHM_SIZE (65536)

int main() {
    managed_shared_memory shm(open_only, SHM_NAME);
    managed_shared_memory::handle_t * handle = shm.find<managed_shared_memory::handle_t>("handle").first;
    // test1: using deallocate
    A * p1 = (A *)shm.get_address_from_handle(handle[0]);
    p1->foo();
    p1->~A();
    shm.deallocate(p1);
    // test2: find anonymous object
    A * p2 = (A *)shm.get_address_from_handle(handle[1]);
    p2->foo();
    shm.destroy_ptr(p2);
    // test3: find named object
    A * p3 = shm.find< A >("A").first;
    p3->foo();
    shm.destroy< A >("A");
    // test4: complex inheritance
    A * p4 = (A *)shm.find< B >("B").first;
    p4->foo();
    shm.destroy< B >("B");
    A * p5 = (A *)shm.find< C >("C").first;
    p5->foo();
    shm.destroy< C >("C");
    A * p6 = (A *)shm.find< D >("D").first;
    p6->foo();
    shm.destroy< D >("D");
    return 0;
}

