// yul
{
    let a:u256
    function f() {
        let b:u256
        function g() { let c:u256}
        let d:u256
    }
}
// ----
// mainFunction
// {
//     function main()
//     {
//         let a:u256
//     }
//     function f()
//     {
//         let b:u256
//         function g()
//         {
//             let c:u256
//         }
//         let d:u256
//     }
// }
