func .test_equal(a #I, b #I) -> #I
    if (a = b) 0 else 1

//* Operators

//* Logic operators
//* There are following logic operators: binary `and`, binary `or`, unary `not`. In any case both branches will be evaluated.

test demo_logic_operators {
    def a := 1 and 1    // now `a` is equal to 1
    def b := 0 and 1    // now `b` is equal to 0
    def c := 0 or 1     // now `c` is equal to 1
    def d := 0 or 0     // now `d` is equal to 0
    def e := not 1      // now `e` is equal to 0
    def f := not 0      // now `f` is equal to 1
    return test_equal(a = 1 and b = 0 and c = 1 and d = 0 and e = 0 and f = 1, 1)
}

//* Bitwise operators
//* There are following bitwise operators: binary and `&`, binary or `|`, binary xor `^`, unary not `~`. In any case both branches will be evaluated.

test demo_bitwise_operators {
    def a := 14 & 7     // now `a` is equal 6
    def b := 9 | 12     // now `b` is equal to 13
    def c := 9 ^ 12     // now `c` is equal to 5
    def d := ~32        // now `d` is equal to -33
    return test_equal(a = 6 and b = 13 and c = 5 and d = -33, 1)
}

//* Operator precendence
//* All operators are left-associative. The operators have following precedence.
//* <table>
//*   <tr>
//*     <th>Precedence</th>
//*     <th>Operators</th>
//*   </tr>
//*   <tr>
//*     <td>2</td>
//*     <td>Unary `-`, `~`, `not`</td>
//*   </tr>
//*   <tr>
//*     <td>3</td>
//*     <td>`*`, `/`, `%`</td>
//*   </tr>
//*   <tr>
//*     <td>4</td>
//*     <td>`+`, `-`</td>
//*   </tr>
//*   <tr>
//*     <td>5</td>
//*     <td>`<<`, `>>`</td>
//*   </tr>
//*   <tr>
//*     <td>6</td>
//*     <td>`<`, `>`, `<=`, `>=`</td>
//*   </tr>
//*   <tr>
//*     <td>7</td>
//*     <td>`=`, `<>`</td>
//*   </tr>
//*   <tr>
//*     <td>8</td>
//*     <td>`&`</td>
//*   </tr>
//*   <tr>
//*     <td>9</td>
//*     <td>`^`</td>
//*   </tr>
//*   <tr>
//*     <td>10</td>
//*     <td>`|`</td>
//*   </tr>
//*   <tr>
//*     <td>11</td>
//*     <td>`and`</td>
//*   </tr>
//*   <tr>
//*     <td>12</td>
//*     <td>`or`</td>
//*   </tr>
//*   <tr>
//*     <td>13</td>
//*     <td>`(`</td>
//*   </tr>
//* </table>
