pub trait AIInterface {
    fn query(&self, input: &str) -> Result<String, String>;
}
