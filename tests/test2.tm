// This is a DTM M that performs computation (s, #w) \--* (h, #u) where u,w in {1,0}*  u is obtained from w by interchanging all 0's with 1's and 1's with 0's in w.

// %include <right>
// %include <left>


alphabet binary = {"0" , "1"};

CTM M ( binary) = {
    ( >, _, R . A ),
    ( A, head == "0" , "1" . R. A . A ),
    ( A, head == "1" , "0" . R . A),
    ( A, head == # , std;;left(#) . h ) // go left until #
};
