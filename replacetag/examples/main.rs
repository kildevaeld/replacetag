use replacetag::*;

fn main() {
    let mut rep = Replacer::new();

    // let p = Pattern::new(':', ':', |input| Some(String::from("Putter")));
    let p = Pattern::new(':', ':', |input| {
        println!("i {}", input);
        Some(String::from("Putter"))
    });

    let mut p2 = Pattern::new('*', '*', |input| Some(format!("<b>{}</b>", input)));

    p2.pattern("[^*]");

    // rep.add_pattern(p);
    // rep.add_pattern(p2);

    let out = rep.replace("Hello, *World* :smiley:");

    println!("result {}", out);
}
