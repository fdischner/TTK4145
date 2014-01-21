// Go 1.2
// go run helloworld_go.go

package main

import (
    . "fmt"     // Using '.' to avoid prefixing functions with their package names
    . "runtime" //   This is probably not a good idea for large projects...
    //. "time"
)

var i = 0

func adder(done chan <- bool, sync chan bool) {
    for x := 0; x < 1000000; x++ {
	<- sync
        i++
        sync <- true
    }
    done <- true
}

func decrement(done chan <- bool, sync chan bool) {
    for x := 0; x < 1000000; x++ {
	<- sync
        i--
        sync <- true
    }
    done <- true
}

func main() {
    GOMAXPROCS(NumCPU())        // I guess this is a hint to what GOMAXPROCS does...
    done := make(chan bool, 2)
    sync := make(chan bool, 1)
    
    sync <- true
    
    go adder(done, sync)                  // This spawns adder() as a goroutine
    go decrement(done, sync)
    for x := 0; x < 50; x++ {
        Println(i)
    }
    // No way to wait for the completion of a goroutine (without additional syncronization)
    // We'll come back to using channels in Exercise 2. For now: Sleep
    //Sleep(100*Millisecond)
    <- done
    <- done
    
    Println("Done:", i);
}