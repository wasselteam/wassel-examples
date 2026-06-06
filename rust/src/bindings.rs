wit_bindgen::generate!({
    path: "../wit",
    world: "http-plugin",
    default_bindings_module: "crate::bindings",
    generate_all,
});
