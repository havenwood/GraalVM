/*
 * Copyright 1997-2009 Sun Microsystems, Inc.  All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
 * CA 95054 USA or visit www.sun.com if you need additional information or
 * have any questions.
 *
 */

class AdapterHandlerEntry;
class vframeStream;

// Runtime is the base class for various runtime interfaces
// (InterpreterRuntime, CompilerRuntime, etc.). It provides
// shared functionality such as exception forwarding (C++ to
// Java exceptions), locking/unlocking mechanisms, statistical
// information, etc.

class SharedRuntime: AllStatic {
 private:
  static methodHandle resolve_sub_helper(JavaThread *thread,
                                     bool is_virtual,
                                     bool is_optimized, TRAPS);

  // Shared stub locations

  static RuntimeStub* _wrong_method_blob;
  static RuntimeStub* _ic_miss_blob;
  static RuntimeStub* _resolve_opt_virtual_call_blob;
  static RuntimeStub* _resolve_virtual_call_blob;
  static RuntimeStub* _resolve_static_call_blob;

  static SafepointBlob* _polling_page_safepoint_handler_blob;
  static SafepointBlob* _polling_page_return_handler_blob;
#ifdef COMPILER2
  static ExceptionBlob*       _exception_blob;
  static UncommonTrapBlob*    _uncommon_trap_blob;
#endif // COMPILER2

#ifndef PRODUCT

  // Counters
  static int     _nof_megamorphic_calls;         // total # of megamorphic calls (through vtable)

#endif // !PRODUCT
 public:

  // max bytes for each dtrace string parameter
  enum { max_dtrace_string_size = 256 };

  // The following arithmetic routines are used on platforms that do
  // not have machine instructions to implement their functionality.
  // Do not remove these.

  // long arithmetics
  static jlong   lmul(jlong y, jlong x);
  static jlong   ldiv(jlong y, jlong x);
  static jlong   lrem(jlong y, jlong x);

  // float and double remainder
  static jfloat  frem(jfloat  x, jfloat  y);
  static jdouble drem(jdouble x, jdouble y);

  // float conversion (needs to set appropriate rounding mode)
  static jint    f2i (jfloat  x);
  static jlong   f2l (jfloat  x);
  static jint    d2i (jdouble x);
  static jlong   d2l (jdouble x);
  static jfloat  d2f (jdouble x);
  static jfloat  l2f (jlong   x);
  static jdouble l2d (jlong   x);

  // double trigonometrics and transcendentals
  static jdouble dsin(jdouble x);
  static jdouble dcos(jdouble x);
  static jdouble dtan(jdouble x);
  static jdouble dlog(jdouble x);
  static jdouble dlog10(jdouble x);
  static jdouble dexp(jdouble x);
  static jdouble dpow(jdouble x, jdouble y);


  // exception handling across interpreter/compiler boundaries
  static address raw_exception_handler_for_return_address(address return_address);
  static address exception_handler_for_return_address(address return_address);

#ifndef SERIALGC
  // G1 write barriers
  static void g1_wb_pre(oopDesc* orig, JavaThread *thread);
  static void g1_wb_post(void* card_addr, JavaThread* thread);
#endif // !SERIALGC

  // exception handling and implicit exceptions
  static address compute_compiled_exc_handler(nmethod* nm, address ret_pc, Handle& exception,
                                              bool force_unwind, bool top_frame_only);
  enum ImplicitExceptionKind {
    IMPLICIT_NULL,
    IMPLICIT_DIVIDE_BY_ZERO,
    STACK_OVERFLOW
  };
  static void    throw_AbstractMethodError(JavaThread* thread);
  static void    throw_IncompatibleClassChangeError(JavaThread* thread);
  static void    throw_ArithmeticException(JavaThread* thread);
  static void    throw_NullPointerException(JavaThread* thread);
  static void    throw_NullPointerException_at_call(JavaThread* thread);
  static void    throw_StackOverflowError(JavaThread* thread);
  static address continuation_for_implicit_exception(JavaThread* thread,
                                                     address faulting_pc,
                                                     ImplicitExceptionKind exception_kind);

  // Shared stub locations
  static address get_poll_stub(address pc);

  static address get_ic_miss_stub() {
    assert(_ic_miss_blob!= NULL, "oops");
    return _ic_miss_blob->instructions_begin();
  }

  static address get_handle_wrong_method_stub() {
    assert(_wrong_method_blob!= NULL, "oops");
    return _wrong_method_blob->instructions_begin();
  }

#ifdef COMPILER2
  static void generate_uncommon_trap_blob(void);
  static UncommonTrapBlob* uncommon_trap_blob()                  { return _uncommon_trap_blob; }
#endif // COMPILER2

  static address get_resolve_opt_virtual_call_stub(){
    assert(_resolve_opt_virtual_call_blob != NULL, "oops");
    return _resolve_opt_virtual_call_blob->instructions_begin();
  }
  static address get_resolve_virtual_call_stub() {
    assert(_resolve_virtual_call_blob != NULL, "oops");
    return _resolve_virtual_call_blob->instructions_begin();
  }
  static address get_resolve_static_call_stub() {
    assert(_resolve_static_call_blob != NULL, "oops");
    return _resolve_static_call_blob->instructions_begin();
  }

  static SafepointBlob* polling_page_return_handler_blob()     { return _polling_page_return_handler_blob; }
  static SafepointBlob* polling_page_safepoint_handler_blob()  { return _polling_page_safepoint_handler_blob; }

  // Counters
#ifndef PRODUCT
  static address nof_megamorphic_calls_addr() { return (address)&_nof_megamorphic_calls; }
#endif // PRODUCT

  // Helper routine for full-speed JVMTI exception throwing support
  static void throw_and_post_jvmti_exception(JavaThread *thread, Handle h_exception);
  static void throw_and_post_jvmti_exception(JavaThread *thread, symbolOop name, const char *message = NULL);

  // RedefineClasses() tracing support for obsolete method entry
  static int rc_trace_method_entry(JavaThread* thread, methodOopDesc* m);

  // To be used as the entry point for unresolved native methods.
  static address native_method_throw_unsatisfied_link_error_entry();

  // bytecode tracing is only used by the TraceBytecodes
  static intptr_t trace_bytecode(JavaThread* thread, intptr_t preserve_this_value, intptr_t tos, intptr_t tos2) PRODUCT_RETURN0;

  // Used to back off a spin lock that is under heavy contention
  static void yield_all(JavaThread* thread, int attempts = 0);

  static oop retrieve_receiver( symbolHandle sig, frame caller );

  static void register_finalizer(JavaThread* thread, oopDesc* obj);

  // dtrace notifications
  static int dtrace_object_alloc(oopDesc* o);
  static int dtrace_object_alloc_base(Thread* thread, oopDesc* o);
  static int dtrace_method_entry(JavaThread* thread, methodOopDesc* m);
  static int dtrace_method_exit(JavaThread* thread, methodOopDesc* m);

  // Utility method for retrieving the Java thread id, returns 0 if the
  // thread is not a well formed Java thread.
  static jlong get_java_tid(Thread* thread);


  // used by native wrappers to reenable yellow if overflow happened in native code
  static void reguard_yellow_pages();

  /**
   * Fill in the "X cannot be cast to a Y" message for ClassCastException
   *
   * @param thr the current thread
   * @param name the name of the class of the object attempted to be cast
   * @return the dynamically allocated exception message (must be freed
   * by the caller using a resource mark)
   *
   * BCP must refer to the current 'checkcast' opcode for the frame
   * on top of the stack.
   * The caller (or one of it's callers) must use a ResourceMark
   * in order to correctly free the result.
   */
  static char* generate_class_cast_message(JavaThread* thr, const char* name);

  /**
   * Fill in the message for a WrongMethodTypeException
   *
   * @param thr the current thread
   * @param mtype (optional) expected method type (or argument class)
   * @param mhandle (optional) actual method handle (or argument)
   * @return the dynamically allocated exception message
   *
   * BCP for the frame on top of the stack must refer to an
   * 'invokevirtual' op for a method handle, or an 'invokedyamic' op.
   * The caller (or one of its callers) must use a ResourceMark
   * in order to correctly free the result.
   */
  static char* generate_wrong_method_type_message(JavaThread* thr,
                                                  oopDesc* mtype = NULL,
                                                  oopDesc* mhandle = NULL);

  /** Return non-null if the mtype is a klass or Class, not a MethodType. */
  static oop wrong_method_type_is_for_single_argument(JavaThread* thr,
                                                      oopDesc* mtype);

  /**
   * Fill in the "X cannot be cast to a Y" message for ClassCastException
   *
   * @param name the name of the class of the object attempted to be cast
   * @param klass the name of the target klass attempt
   * @param gripe the specific kind of problem being reported
   * @return the dynamically allocated exception message (must be freed
   * by the caller using a resource mark)
   *
   * This version does not require access the frame, so it can be called
   * from interpreted code
   * The caller (or one of it's callers) must use a ResourceMark
   * in order to correctly free the result.
   */
  static char* generate_class_cast_message(const char* name, const char* klass,
                                           const char* gripe = " cannot be cast to ");

  // Resolves a call site- may patch in the destination of the call into the
  // compiled code.
  static methodHandle resolve_helper(JavaThread *thread,
                                     bool is_virtual,
                                     bool is_optimized, TRAPS);

  static void generate_stubs(void);

  private:
  // deopt blob
  static void generate_deopt_blob(void);
  static DeoptimizationBlob* _deopt_blob;

  public:
  static DeoptimizationBlob* deopt_blob(void)      { return _deopt_blob; }

  // Resets a call-site in compiled code so it will get resolved again.
  static methodHandle reresolve_call_site(JavaThread *thread, TRAPS);

  // In the code prolog, if the klass comparison fails, the inline cache
  // misses and the call site is patched to megamorphic
  static methodHandle handle_ic_miss_helper(JavaThread* thread, TRAPS);

  // Find the method that called us.
  static methodHandle find_callee_method(JavaThread* thread, TRAPS);


 private:
  static Handle find_callee_info(JavaThread* thread,
                                 Bytecodes::Code& bc,
                                 CallInfo& callinfo, TRAPS);
  static Handle find_callee_info_helper(JavaThread* thread,
                                        vframeStream& vfst,
                                        Bytecodes::Code& bc,
                                        CallInfo& callinfo, TRAPS);

  static address clean_virtual_call_entry();
  static address clean_opt_virtual_call_entry();
  static address clean_static_call_entry();

 public:

  // Read the array of BasicTypes from a Java signature, and compute where
  // compiled Java code would like to put the results.  Values in reg_lo and
  // reg_hi refer to 4-byte quantities.  Values less than SharedInfo::stack0 are
  // registers, those above refer to 4-byte stack slots.  All stack slots are
  // based off of the window top.  SharedInfo::stack0 refers to the first usable
  // slot in the bottom of the frame. SharedInfo::stack0+1 refers to the memory word
  // 4-bytes higher. So for sparc because the register window save area is at
  // the bottom of the frame the first 16 words will be skipped and SharedInfo::stack0
  // will be just above it. (
  // return value is the maximum number of VMReg stack slots the convention will use.
  static int java_calling_convention(const BasicType *sig_bt, VMRegPair *regs, int total_args_passed, int is_outgoing);

  // Ditto except for calling C
  static int c_calling_convention(const BasicType *sig_bt, VMRegPair *regs, int total_args_passed);

  // Generate I2C and C2I adapters. These adapters are simple argument marshalling
  // blobs. Unlike adapters in the tiger and earlier releases the code in these
  // blobs does not create a new frame and are therefore virtually invisible
  // to the stack walking code. In general these blobs extend the callers stack
  // as needed for the conversion of argument locations.

  // When calling a c2i blob the code will always call the interpreter even if
  // by the time we reach the blob there is compiled code available. This allows
  // the blob to pass the incoming stack pointer (the sender sp) in a known
  // location for the interpreter to record. This is used by the frame code
  // to correct the sender code to match up with the stack pointer when the
  // thread left the compiled code. In addition it allows the interpreter
  // to remove the space the c2i adapter allocated to do it argument conversion.

  // Although a c2i blob will always run interpreted even if compiled code is
  // present if we see that compiled code is present the compiled call site
  // will be patched/re-resolved so that later calls will run compiled.

  // Aditionally a c2i blob need to have a unverified entry because it can be reached
  // in situations where the call site is an inlined cache site and may go megamorphic.

  // A i2c adapter is simpler than the c2i adapter. This is because it is assumed
  // that the interpreter before it does any call dispatch will record the current
  // stack pointer in the interpreter frame. On return it will restore the stack
  // pointer as needed. This means the i2c adapter code doesn't need any special
  // handshaking path with compiled code to keep the stack walking correct.

  static AdapterHandlerEntry* generate_i2c2i_adapters(MacroAssembler *_masm,
                                                      int total_args_passed,
                                                      int max_arg,
                                                      const BasicType *sig_bt,
                                                      const VMRegPair *regs);

  // OSR support

  // OSR_migration_begin will extract the jvm state from an interpreter
  // frame (locals, monitors) and store the data in a piece of C heap
  // storage. This then allows the interpreter frame to be removed from the
  // stack and the OSR nmethod to be called. That method is called with a
  // pointer to the C heap storage. This pointer is the return value from
  // OSR_migration_begin.

  static intptr_t* OSR_migration_begin( JavaThread *thread);

  // OSR_migration_end is a trivial routine. It is called after the compiled
  // method has extracted the jvm state from the C heap that OSR_migration_begin
  // created. It's entire job is to simply free this storage.
  static void      OSR_migration_end  ( intptr_t* buf);

  // Convert a sig into a calling convention register layout
  // and find interesting things about it.
  static VMRegPair* find_callee_arguments(symbolOop sig, bool has_receiver, int *arg_size);
  static VMReg     name_for_receiver();

  // "Top of Stack" slots that may be unused by the calling convention but must
  // otherwise be preserved.
  // On Intel these are not necessary and the value can be zero.
  // On Sparc this describes the words reserved for storing a register window
  // when an interrupt occurs.
  static uint out_preserve_stack_slots();

  // Save and restore a native result
  static void    save_native_result(MacroAssembler *_masm, BasicType ret_type, int frame_slots );
  static void restore_native_result(MacroAssembler *_masm, BasicType ret_type, int frame_slots );

  // Generate a native wrapper for a given method.  The method takes arguments
  // in the Java compiled code convention, marshals them to the native
  // convention (handlizes oops, etc), transitions to native, makes the call,
  // returns to java state (possibly blocking), unhandlizes any result and
  // returns.
  static nmethod *generate_native_wrapper(MacroAssembler* masm,
                                          methodHandle method,
                                          int total_args_passed,
                                          int max_arg,
                                          BasicType *sig_bt,
                                          VMRegPair *regs,
                                          BasicType ret_type );

#ifdef HAVE_DTRACE_H
  // Generate a dtrace wrapper for a given method.  The method takes arguments
  // in the Java compiled code convention, marshals them to the native
  // convention (handlizes oops, etc), transitions to native, makes the call,
  // returns to java state (possibly blocking), unhandlizes any result and
  // returns.
  static nmethod *generate_dtrace_nmethod(MacroAssembler* masm,
                                          methodHandle method);

  // dtrace support to convert a Java string to utf8
  static void get_utf(oopDesc* src, address dst);
#endif // def HAVE_DTRACE_H

  // A compiled caller has just called the interpreter, but compiled code
  // exists.  Patch the caller so he no longer calls into the interpreter.
  static void fixup_callers_callsite(methodOopDesc* moop, address ret_pc);

  // Slow-path Locking and Unlocking
  static void complete_monitor_locking_C(oopDesc* obj, BasicLock* lock, JavaThread* thread);
  static void complete_monitor_unlocking_C(oopDesc* obj, BasicLock* lock);

  // Resolving of calls
  static address resolve_static_call_C     (JavaThread *thread);
  static address resolve_virtual_call_C    (JavaThread *thread);
  static address resolve_opt_virtual_call_C(JavaThread *thread);

  // arraycopy, the non-leaf version.  (See StubRoutines for all the leaf calls.)
  static void slow_arraycopy_C(oopDesc* src,  jint src_pos,
                               oopDesc* dest, jint dest_pos,
                               jint length, JavaThread* thread);

  // handle ic miss with caller being compiled code
  // wrong method handling (inline cache misses, zombie methods)
  static address handle_wrong_method(JavaThread* thread);
  static address handle_wrong_method_ic_miss(JavaThread* thread);

#ifndef PRODUCT

  // Collect and print inline cache miss statistics
 private:
  enum { maxICmiss_count = 100 };
  static int     _ICmiss_index;                  // length of IC miss histogram
  static int     _ICmiss_count[maxICmiss_count]; // miss counts
  static address _ICmiss_at[maxICmiss_count];    // miss addresses
  static void trace_ic_miss(address at);

 public:
  static int _monitor_enter_ctr;                 // monitor enter slow
  static int _monitor_exit_ctr;                  // monitor exit slow
  static int _throw_null_ctr;                    // throwing a null-pointer exception
  static int _ic_miss_ctr;                       // total # of IC misses
  static int _wrong_method_ctr;
  static int _resolve_static_ctr;
  static int _resolve_virtual_ctr;
  static int _resolve_opt_virtual_ctr;
  static int _implicit_null_throws;
  static int _implicit_div0_throws;

  static int _jbyte_array_copy_ctr;        // Slow-path byte array copy
  static int _jshort_array_copy_ctr;       // Slow-path short array copy
  static int _jint_array_copy_ctr;         // Slow-path int array copy
  static int _jlong_array_copy_ctr;        // Slow-path long array copy
  static int _oop_array_copy_ctr;          // Slow-path oop array copy
  static int _checkcast_array_copy_ctr;    // Slow-path oop array copy, with cast
  static int _unsafe_array_copy_ctr;       // Slow-path includes alignment checks
  static int _generic_array_copy_ctr;      // Slow-path includes type decoding
  static int _slow_array_copy_ctr;         // Slow-path failed out to a method call

  static int _new_instance_ctr;            // 'new' object requires GC
  static int _new_array_ctr;               // 'new' array requires GC
  static int _multi1_ctr, _multi2_ctr, _multi3_ctr, _multi4_ctr, _multi5_ctr;
  static int _find_handler_ctr;            // find exception handler
  static int _rethrow_ctr;                 // rethrow exception
  static int _mon_enter_stub_ctr;          // monitor enter stub
  static int _mon_exit_stub_ctr;           // monitor exit stub
  static int _mon_enter_ctr;               // monitor enter slow
  static int _mon_exit_ctr;                // monitor exit slow
  static int _partial_subtype_ctr;         // SubRoutines::partial_subtype_check

  // Statistics code
  // stats for "normal" compiled calls (non-interface)
  static int     _nof_normal_calls;              // total # of calls
  static int     _nof_optimized_calls;           // total # of statically-bound calls
  static int     _nof_inlined_calls;             // total # of inlined normal calls
  static int     _nof_static_calls;              // total # of calls to static methods or super methods (invokespecial)
  static int     _nof_inlined_static_calls;      // total # of inlined static calls
  // stats for compiled interface calls
  static int     _nof_interface_calls;           // total # of compiled calls
  static int     _nof_optimized_interface_calls; // total # of statically-bound interface calls
  static int     _nof_inlined_interface_calls;   // total # of inlined interface calls
  static int     _nof_megamorphic_interface_calls;// total # of megamorphic interface calls
  // stats for runtime exceptions
  static int     _nof_removable_exceptions;      // total # of exceptions that could be replaced by branches due to inlining

 public: // for compiler
  static address nof_normal_calls_addr()                { return (address)&_nof_normal_calls; }
  static address nof_optimized_calls_addr()             { return (address)&_nof_optimized_calls; }
  static address nof_inlined_calls_addr()               { return (address)&_nof_inlined_calls; }
  static address nof_static_calls_addr()                { return (address)&_nof_static_calls; }
  static address nof_inlined_static_calls_addr()        { return (address)&_nof_inlined_static_calls; }
  static address nof_interface_calls_addr()             { return (address)&_nof_interface_calls; }
  static address nof_optimized_interface_calls_addr()   { return (address)&_nof_optimized_interface_calls; }
  static address nof_inlined_interface_calls_addr()     { return (address)&_nof_inlined_interface_calls; }
  static address nof_megamorphic_interface_calls_addr() { return (address)&_nof_megamorphic_interface_calls; }
  static void print_call_statistics(int comp_total);
  static void print_statistics();
  static void print_ic_miss_histogram();

#endif // PRODUCT
};


// ---------------------------------------------------------------------------
// Implementation of AdapterHandlerLibrary
//
// This library manages argument marshaling adapters and native wrappers.
// There are 2 flavors of adapters: I2C and C2I.
//
// The I2C flavor takes a stock interpreted call setup, marshals the arguments
// for a Java-compiled call, and jumps to Rmethod-> code()->
// instructions_begin().  It is broken to call it without an nmethod assigned.
// The usual behavior is to lift any register arguments up out of the stack
// and possibly re-pack the extra arguments to be contigious.  I2C adapters
// will save what the interpreter's stack pointer will be after arguments are
// popped, then adjust the interpreter's frame size to force alignment and
// possibly to repack the arguments.  After re-packing, it jumps to the
// compiled code start.  There are no safepoints in this adapter code and a GC
// cannot happen while marshaling is in progress.
//
// The C2I flavor takes a stock compiled call setup plus the target method in
// Rmethod, marshals the arguments for an interpreted call and jumps to
// Rmethod->_i2i_entry.  On entry, the interpreted frame has not yet been
// setup.  Compiled frames are fixed-size and the args are likely not in the
// right place.  Hence all the args will likely be copied into the
// interpreter's frame, forcing that frame to grow.  The compiled frame's
// outgoing stack args will be dead after the copy.
//
// Native wrappers, like adapters, marshal arguments.  Unlike adapters they
// also perform an offical frame push & pop.  They have a call to the native
// routine in their middles and end in a return (instead of ending in a jump).
// The native wrappers are stored in real nmethods instead of the BufferBlobs
// used by the adapters.  The code generation happens here because it's very
// similar to what the adapters have to do.

class AdapterHandlerEntry : public CHeapObj {
 private:
  address _i2c_entry;
  address _c2i_entry;
  address _c2i_unverified_entry;

 public:

  // The name we give all buffer blobs
  static const char* name;

  AdapterHandlerEntry(address i2c_entry, address c2i_entry, address c2i_unverified_entry):
    _i2c_entry(i2c_entry),
    _c2i_entry(c2i_entry),
    _c2i_unverified_entry(c2i_unverified_entry) {
  }

  address get_i2c_entry()            { return _i2c_entry; }
  address get_c2i_entry()            { return _c2i_entry; }
  address get_c2i_unverified_entry() { return _c2i_unverified_entry; }

  void relocate(address new_base);
#ifndef PRODUCT
  void print();
#endif /* PRODUCT */
};

class AdapterHandlerLibrary: public AllStatic {
 private:
  static BufferBlob* _buffer; // the temporary code buffer in CodeCache
  static GrowableArray<uint64_t>* _fingerprints; // the fingerprint collection
  static GrowableArray<AdapterHandlerEntry*> * _handlers; // the corresponding handlers
  enum {
    AbstractMethodHandler = 1 // special handler for abstract methods
  };
  static BufferBlob* buffer_blob();
  static void initialize();
  static int get_create_adapter_index(methodHandle method);
  static address get_i2c_entry( int index ) {
    return get_entry(index)->get_i2c_entry();
  }
  static address get_c2i_entry( int index ) {
    return get_entry(index)->get_c2i_entry();
  }
  static address get_c2i_unverified_entry( int index ) {
    return get_entry(index)->get_c2i_unverified_entry();
  }

 public:
  static AdapterHandlerEntry* get_entry( int index ) { return _handlers->at(index); }
  static nmethod* create_native_wrapper(methodHandle method);
  static AdapterHandlerEntry* get_adapter(methodHandle method)  {
    return get_entry(get_create_adapter_index(method));
  }
#ifdef HAVE_DTRACE_H
  static nmethod* create_dtrace_nmethod (methodHandle method);
#endif // HAVE_DTRACE_H

#ifndef PRODUCT
  static void print_handler(CodeBlob* b);
  static bool contains(CodeBlob* b);
#endif /* PRODUCT */

};
