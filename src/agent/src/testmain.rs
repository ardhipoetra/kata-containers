use nix::fcntl::OFlag;
use std::fs; 
use std::os::unix::io::{FromRawFd, AsRawFd};
use std::convert::TryFrom;
// use std::convert::TryInto;

use ed25519_dalek::{VerifyingKey, Signature};
use sha2::{Sha256, Digest};

pub mod scbindings {
    #![allow(warnings)] 

    include!(concat!(env!("OUT_DIR"), "/bindings.rs"));
}

fn main() -> std::result::Result<(), Box<dyn std::error::Error>> {
    // do SCONE_IOC_DICE
    let mut args = scbindings::scone_dice {
        cdi: alloc_buffer(32),
        out: scbindings::scone_cert_t { 
            body: &mut scbindings::scone_cert_body_t {
                author_pubkey: [0u8;32],
                subject_pubkey: [0u8;32],
                measurement: [0u8;32],
            } as *mut scbindings::scone_cert_body_t,
            cert_signature: alloc_buffer(64),
        },
    }; 

    nix::ioctl_read!(sc_dice, b'a', 13, scbindings::scone_dice);

    let f = {
        let fd = nix::fcntl::open("/dev/scone_enclave", OFlag::O_RDONLY, nix::sys::stat::Mode::all());
        // Wrap fd with `File` to properly close descriptor on exit
        unsafe { fs::File::from_raw_fd(fd.expect("fd errr")) }
    };
    println!("Call ioctl SCONE_IOC_DICE");
    
    unsafe {
        let ret = sc_dice(f.as_raw_fd(), &mut args);
        println!("ioctl SCONE_IOC_DICE return : {:?}", ret);
    }

    // verify the cert
    let body = unsafe {(*args.out.body).clone()};
    let author_public_key = VerifyingKey::from_bytes( &body.author_pubkey )
        .expect("Verif-key from_bytes fails");


    println!("author_public_key: {:?}", hex(&body.author_pubkey));
    println!("subject_public_key: {:?}", hex(&body.subject_pubkey));
    println!("measurement: {:?}", hex(&body.measurement));

    let signature = Signature::from_bytes( unsafe {
        &mut *(args.out.cert_signature as *mut [u8; 64])
    });
    let slice_sign = unsafe {std::slice::from_raw_parts(args.out.cert_signature, 64)};
    println!("signature: {:?}", hex(&<[u8; 64]>::try_from(slice_sign).expect("Slice length must match array size")));
    
    let body_v: Vec<u8> = [body.author_pubkey, body.subject_pubkey, body.measurement].concat();
    
    let ret = author_public_key.verify_strict(&body_v, &signature);
    match ret {
        Ok(_) => {
            println!("Cert signature OK");
            println!("The cert is signed by {:?}", hex(&body.author_pubkey));
        },
        Err(ref e) => println!("Cert verification failed: {:?}", e),
    }
    // end SCONE_IOC_DICE
    
    let slice_cdi = unsafe {std::slice::from_raw_parts(args.cdi, 32)};
    println!("\n***Connecting the CDI***\n");

    let cdi_kata = "6986219f81b895e17b3e8266ad4231047cfdddbbb01d80eaaf8d439b156b0726";

    println!("Suppose the KA CDI is (hardcoded): {}", cdi_kata);

    let concat_str = format!("{}{}", hex(&body.measurement), cdi_kata);
    println!("Concat it with my measurement: {:?}", concat_str);

    let mut concat_arr = [0u8; 64];
    for i in 0..64 {
        concat_arr[i] = u8::from_str_radix(&concat_str[i*2..i*2+2], 16).ok().expect("Invalid hex");
    }

    let mut hasher = Sha256::new();
    hasher.update(&concat_arr);
    let result = hasher.finalize();

    println!("SHA256 hash\t: {:x}", result);
    println!("my CDI\t\t: {}", hex(&<[u8; 32]>::try_from(slice_cdi).expect("Slice length must match array size")));

    Ok(())
}

fn hex<const N: usize>(data: &[u8; N]) -> String {
    data.iter()
        .map(|byte| format!("{:02x}", byte))
        .collect::<String>()
}

fn alloc_buffer(size: usize) -> *mut u8 {
    let ptr = unsafe { libc::malloc(size) };
    unsafe { ptr.write_bytes(0, size) }; // memset(0)
    ptr as *mut u8
}