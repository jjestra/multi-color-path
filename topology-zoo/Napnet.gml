graph [
  label "Napnet"
  node [
    id 0
    label "Seattle"
    graphics
    [
     x -122.33207
     y 47.60621
    ]
  ]
  node [
    id 1
    label "San Jose"
    graphics
    [
     x -121.89496
     y 37.33939
    ]
  ]
  node [
    id 2
    label "Minneapolis"
    graphics
    [
     x -93.26384
     y 44.97997
    ]
  ]
  node [
    id 3
    label "Chicago"
    graphics
    [
     x -87.65005
     y 41.85003
    ]
  ]
  node [
    id 4
    label "Vienna"
    graphics
    [
     x -77.26526
     y 38.90122
    ]
  ]
  node [
    id 5
    label "Dallas"
    graphics
    [
     x -96.80667
     y 32.78306
    ]
  ]
  edge [
    source 0
    target 1
    id "e0"
  ]
  edge [
    source 0
    target 3
    id "e1"
  ]
  edge [
    source 1
    target 3
    id "e2"
  ]
  edge [
    source 1
    target 4
  ]
  edge [
    source 2
    target 3
    id "e4"
  ]
  edge [
    source 3
    target 4
    id "e5"
  ]
  edge [
    source 3
    target 5
    id "e6"
  ]
]
