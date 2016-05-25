use std::thread;
use std::io::Write;
use std::net::*;
use std::io::Result;

fn handle_client(mut stream: TcpStream){
    let response = b"HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\ncontent-length: 21\r\n\r\n<h1>Test content</h1>";
    
    match stream.write(response){
        Ok(written) => println!("printed {} bytes", written),
        Err(e) => println!("failed sending, err:{}", e),
    }
    
    drop(stream);
}

fn server(address : & str) -> Result<TcpListener>{
    let listener : Result<TcpListener> = 
        TcpListener::bind(address);

    let listener = match listener {
        Ok(listener) =>  {
            println!("listener ok");
            listener
        },
        Err(err) => return Err(err),
    };

    match listener.local_addr() {
        Ok(socket_address) => {
            println!("we are using socket address {}", socket_address);
            socket_address
        },
        Err(e) => return Err(e),
    };

    for stream in listener.incoming() {
        match stream {
            Err(e) => {
                println!("failure in incoming stream()");
                return Err(e);
            }
            Ok(stream) => {
                thread::spawn(move || handle_client(stream));
            }
        }
    }

    Ok(listener)
}

fn main(){
   match server("localhost:2047") {
        Ok(_) => println!("no problems found"),
        Err(e) => panic!(e),
   }
}
