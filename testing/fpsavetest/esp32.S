    .macro load_one p i
    l32i    a3, \p, \i * 4
    wfr     f\i, a3
    .endm

    .macro save_one p i
    rfr     a3, f\i
    s32i    a3, \p, \i * 4
    .endm

    .global fpsavetest_load
    .type   fpsavetest_load, @function

fpsavetest_load:
    entry   a1, 16
    load_one a2 0
    load_one a2 1
    load_one a2 2
    load_one a2 3
    load_one a2 4
    load_one a2 5
    load_one a2 6
    load_one a2 7
    load_one a2 8
    load_one a2 9
    load_one a2 10
    load_one a2 11
    load_one a2 12
    load_one a2 13
    load_one a2 14
    load_one a2 15
    retw.n

    .global fpsavetest_save
    .type   fpsavetest_save, @function

    .align 4
fpsavetest_save:
    entry   a1, 16
    save_one a2 0
    save_one a2 1
    save_one a2 2
    save_one a2 3
    save_one a2 4
    save_one a2 5
    save_one a2 6
    save_one a2 7
    save_one a2 8
    save_one a2 9
    save_one a2 10
    save_one a2 11
    save_one a2 12
    save_one a2 13
    save_one a2 14
    save_one a2 15
    retw.n
