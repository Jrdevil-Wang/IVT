#include <iostream>
#include <boost/interprocess/managed_shared_memory.hpp>
#include "cls.hpp"

using namespace std;
using namespace boost::interprocess;
using namespace NN::MM;

#define SHM_NAME "test_shm"
#define SHM_SIZE (65536)

int main() {
    // clean shm
    struct shm_remove {
        shm_remove()  { shared_memory_object::remove(SHM_NAME); }
        ~shm_remove() { shared_memory_object::remove(SHM_NAME); }
    } remover;
    // prepare shm
    managed_shared_memory shm(create_only, SHM_NAME, SHM_SIZE);
    managed_shared_memory::handle_t * handle = shm.construct<managed_shared_memory::handle_t>("handle")[2]();
    // test1: using allocate
    A * p1 = (A *)shm.allocate(sizeof(A));
    new(p1) A(1);
    handle[0] = shm.get_handle_from_address(p1);
    // test2: construct anonymous
    A * p2 = shm.construct< A >(anonymous_instance)(2);
    handle[1] = shm.get_handle_from_address(p2);
    // test3: construct named object
    A * p3 = shm.construct< A >("A")(3);
    // test4: complex inheritance
    A * p4 = (A *)shm.construct< B >("B")(4);
    A * p5 = (A *)shm.construct< C >("C")(5);
    A * p6 = (A *)shm.construct< D >("D")(6);
    // wait for reciever
    getchar();
    return 0;
}

