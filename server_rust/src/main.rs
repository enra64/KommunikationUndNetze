use std::net::{TcpListener, TcpStream};
use std::thread;
use std::io::Write;

fn handle_client(mut stream: TcpStream){
    let response = b"HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\ncontent-length: 21\r\n\r\n<h1>Test content</h1>";
    
    match stream.write(response){
        Ok(written) => println!("printed {} bytes", written),
        Err(e) => println!("failed sending, err:{}", e),
    }
    
    drop(stream);
}

fn main(){
    let listener = TcpListener::bind("localhost:2047").unwrap();

//    let mut acceptor = listener.listen();
    
    println!("listening on port 2047");
    
    for stream in listener.incoming(){
        match stream {
            Ok(stream) => {
                thread::spawn(move || {
                    handle_client(stream);
                });
            }
            Err(e) => {
                println!("connection failed, err:{}", e);
            }
        }
    }
}
