//
// Bareflank Hypervisor
// Copyright (C) 2015 Assured Information Security, Inc.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

#include <iostream>

#include <bfaffinity.h>
#include <intrinsics.h>

// The goal of this userspace application is to output "hello world" to the
// console three times, but on the second time, tell the hypervisor to hook
// our call to output "hello world" to instead output "hooked hello world".

void
hello_world()
{ std::clog << "hello world" << '\n'; }

void
hooked_hello_world()
{ std::clog << "hooked hello world" << '\n'; }

int main()
{
    // We don't do core synchronization in the hypervisor for modifying EPT
    // which means that its possible (unlikely) for a CPU race to occur. To
    // prevent this, we tell our application to only run on CPU 0. To remove
    // this, EPT would need to be set up on all cores, and modifications to
    // EPT would require core synchronization, while requires IPIs that are
    // trapped by the hypervisor and processed properly. This can be done
    // using the EAPIs (all of the APIs are available), but it would make the
    // example far more complicated.
    //
    set_affinity(0);

    // Output "hello world" to the console to show that it works as expected.
    // In this case, we expect "hello world" to be outputted.
    //
    hello_world();
    hello_world();

    // The following calls into the hypervisor to tell the hypervisor to hook
    // our hello_world() function. Once this call is made, all attempts to
    // call hello_world() will result in hooked_hello_world() being called
    // instead.
    //
    ::intel_x64::vm::call(
        0,
        reinterpret_cast<uintptr_t>(hello_world),
        reinterpret_cast<uintptr_t>(hooked_hello_world)
    );

    // Attempt to call "hello world". If the hypervisor has done its job,
    // hooked_hello_world will be called instead.
    //
    hello_world();
    hello_world();

    // The following tells the hypervisor to unhook our function. This is
    // important because the hypervisor is currently hooking a guest physical
    // address, so once the application is done executing, it will continue
    // to hook the same guest physical address being used by other applications
    // which could resulting in undefined behaviour.
    //
    ::intel_x64::vm::call(
        1
    );

    // Output "hello world" to the console to show that it works as expected.
    // In this case, we expect "hello world" to be outputted.
    //
    hello_world();
    hello_world();

    // Add a newline to the console to keep the output clean.
    //
    std::clog << '\n';
}
