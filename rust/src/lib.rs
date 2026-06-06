mod bindings;

use bindings::exports::wassel::foundation::http_handler::Guest;

use crate::bindings::{
    export,
    wasi::http::types::{
        Fields, IncomingRequest, OutgoingBody, OutgoingResponse, ResponseOutparam,
    },
};

struct Plugin;

impl Guest for Plugin {
    fn handle_request(request: IncomingRequest, response_out: ResponseOutparam) {
        let path = request.path_with_query().unwrap_or("/".to_owned());
        let content = format!("Hello, {path}!").as_bytes().to_vec();

        let fields = Fields::from_list(&[(
            "content-length".into(),
            format!("{}", content.len()).as_bytes().to_vec(),
        )])
        .expect("Could not create fields");

        let out = OutgoingResponse::new(fields);
        let body = out.body().expect("Could not get body");
        {
            let stream = body.write().expect("Could not get stream");
            stream
                .blocking_write_and_flush(&content)
                .expect("Could not write to stream");
            drop(stream);
        }
        OutgoingBody::finish(body, None).expect("Could not finish body");
        ResponseOutparam::set(response_out, Ok(out));
    }
}

export!(Plugin);
