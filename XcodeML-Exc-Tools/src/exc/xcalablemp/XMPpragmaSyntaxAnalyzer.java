/*
 * $TSUKUBA_Release: $
 * $TSUKUBA_Copyright:
 *  $
 */

package exc.xcalablemp;

import xcodeml.XmException;
import xcodeml.util.XmOption;
import exc.object.*;
import exc.openmp.OMPpragma;
import static exc.object.PragmaLexer.*;
import exc.xcodeml.XmSymbol;
import exc.xcodeml.XmSymbolUtil;

/**
 * XcalableMP pragma lexer
 */
public class XMPpragmaSyntaxAnalyzer implements ExternalPragmaLexer {
  private PragmaLexer _lexer;
    
  public XMPpragmaSyntaxAnalyzer(PragmaLexer lexer) {
    _lexer = lexer;
  }

  @Override
  public Result continueLex() throws XmException {
    XobjList[] retArgs = { null };
    PragmaSyntax[] retSyntax = { null };
    XMPpragma pragmaDir = null;

    try {
      pragmaDir = lexXMPdirective(retSyntax, retArgs);
    } catch (XMPexception e) {
      String message = e.getMessage();
      if (message != null) System.out.println("[XcalableMP] " + message);

      return new Result(_lexer, null);
    }

    XobjList xobj = null;
    if (retSyntax[0] == PragmaSyntax.SYN_PREFIX) {
      xobj = Xcons.List(Xcode.XMP_PRAGMA,
                        Xcons.String(retSyntax[0].toString()),
                        Xcons.String(pragmaDir.toString()),
                        retArgs[0]);
    }
    else {
      xobj = Xcons.List(Xcode.XMP_PRAGMA,
                        Xcons.String(pragmaDir.toString()),
                        retArgs[0],
                        null);
      xobj.setIsParsed(true);
    }

    return new Result(_lexer, xobj);
  }

  private boolean pg_is_ident(String name) {
    return _lexer.pg_is_ident(name);
  }

  private void pg_get_token() {
    _lexer.pg_get_token();
  }

  private char pg_tok() {
    return _lexer.pg_tok();
  }

  private String pg_tok_buf() {
    return _lexer.pg_tok_buf();
  }

  private Xobject pg_parse_expr() throws XmException {
    return _lexer.pg_parse_expr();
  }

  private XobjList pg_parse_int_triplet(boolean allowIntExpr) throws XmException, XMPexception {
    Xobject lower = null, upper = null, stride = null;

    // parse <triplet-lower>
    if (pg_tok() == ':') lower = null;
    else {
      lower = pg_parse_int_expr();

      if (allowIntExpr) {
        if (pg_tok() == ',' || pg_tok() == ')') {
          upper = lower;
          stride = null;
          return Xcons.List(lower, upper, stride);
        }
      }

      if (pg_tok() != ':')
        error(": is expected after <triplet-stride>");
    }

    // parse <triplet-upper>
    pg_get_token();
    if (pg_tok() == ',' || pg_tok() == ')') {
      upper = null;
      stride = null;
    }
    else if (pg_tok() == ':') {
      upper = null;

      pg_get_token();
      stride = pg_parse_int_expr();
    }
    else {
      upper = pg_parse_int_expr();

      if (pg_tok() == ':') {
        pg_get_token();
        stride = pg_parse_int_expr();
      }
      else if (pg_tok() == ',' || pg_tok() == ')') stride = null;
      else
        error("syntax error on <triplet>");
    }

    return Xcons.List(lower, upper, stride);
  }

  private Xobject pg_parse_int_expr() throws XmException, XMPexception {
    Xobject x = _lexer.pg_parse_expr();
    if (x == null)
      error("syntax error on <int-expr>");
    
    if (!XMPutil.isIntegerType(x.Type()))
	if(x.Type().getRef() != Xtype.intType )  // x is a pointer refence
	    error("<int-expr> is expected, parsed expression has a non-integer type");

    return x;
  }

  private void error(String s) throws XMPexception {
    _lexer.error("[XcalableMP] " + s);
    throw new XMPexception();
  }

  private XMPpragma lexXMPdirective(PragmaSyntax[] retSyntax, XobjList[] retArgs) throws XmException, XMPexception {
    PragmaSyntax syntax = null;
    XobjList args = null;
    XMPpragma pragmaDir = null;

    pg_get_token();
    if (pg_tok() != PG_IDENT)
      error("unknown XcalableMP directive, '" + pg_tok_buf() + "'");

    if (pg_is_ident("nodes")) {
      pragmaDir = XMPpragma.NODES;
      syntax = PragmaSyntax.SYN_DECL;

      pg_get_token();
      args = parse_NODES_clause();
    }
    else if (pg_is_ident("template")) {
      pragmaDir = XMPpragma.TEMPLATE;
      syntax = PragmaSyntax.SYN_DECL;

      pg_get_token();
      args = parse_TEMPLATE_clause();
    }
    else if (pg_is_ident("distribute")) {
      pragmaDir = XMPpragma.DISTRIBUTE;
      syntax = PragmaSyntax.SYN_DECL;

      pg_get_token();
      args = parse_DISTRIBUTE_clause();
    }
    else if (pg_is_ident("align")) {
      pragmaDir = XMPpragma.ALIGN;
      syntax = PragmaSyntax.SYN_DECL;

      pg_get_token();
      args = parse_ALIGN_clause();
    }
    else if (pg_is_ident("shadow")) {
      pragmaDir = XMPpragma.SHADOW;
      syntax = PragmaSyntax.SYN_DECL;

      pg_get_token();
      args = parse_SHADOW_clause();
    }
    else if (pg_is_ident("task")) {
      pragmaDir = XMPpragma.TASK;
      syntax = PragmaSyntax.SYN_PREFIX;

      pg_get_token();
      args = parse_TASK_clause();
    }
    else if (pg_is_ident("tasks")) {
      pragmaDir = XMPpragma.TASKS;
      syntax = PragmaSyntax.SYN_PREFIX;

      pg_get_token();
      args = parse_TASKS_clause();
    }
    else if (pg_is_ident("loop")) {
      pragmaDir = XMPpragma.LOOP;
      syntax = PragmaSyntax.SYN_PREFIX;

      pg_get_token();
      args = parse_LOOP_clause(false);
    }
    else if (pg_is_ident("reflect")) {
      pragmaDir = XMPpragma.REFLECT;
      syntax = PragmaSyntax.SYN_EXEC;

      pg_get_token();
      args = parse_REFLECT_clause();
    }
    else if (pg_is_ident("barrier")) {
      pragmaDir = XMPpragma.BARRIER;
      syntax = PragmaSyntax.SYN_EXEC;

      pg_get_token();
      args = parse_BARRIER_clause();
    }
    else if (pg_is_ident("reduction")) {
      pragmaDir = XMPpragma.REDUCTION;
      syntax = PragmaSyntax.SYN_EXEC;

      pg_get_token();
      args = parse_REDUCTION_clause();
    }
    else if (pg_is_ident("bcast")) {
      pragmaDir = XMPpragma.BCAST;
      syntax = PragmaSyntax.SYN_EXEC;

      pg_get_token();
      args = parse_BCAST_clause();
    }
    else if (pg_is_ident("gmove")) {
      pragmaDir = XMPpragma.GMOVE;
      syntax = PragmaSyntax.SYN_PREFIX;

      pg_get_token();
      args = parse_GMOVE_clause();
    }
    else if (pg_is_ident("coarray")) {
      pragmaDir = XMPpragma.COARRAY;
      syntax = PragmaSyntax.SYN_DECL;

      pg_get_token();
      args = parse_COARRAY_clause();
    } else if (pg_is_ident("sync_memory")) {
      pragmaDir = XMPpragma.SYNC_MEMORY;
      syntax = PragmaSyntax.SYN_EXEC;

      pg_get_token();
      args = null;
    } else if (pg_is_ident("sync_all")) {
      pragmaDir = XMPpragma.SYNC_ALL;
      syntax = PragmaSyntax.SYN_EXEC;

      pg_get_token();
      args = null;
    }else if (pg_is_ident("post")) {
      pragmaDir = XMPpragma.POST;
      syntax = PragmaSyntax.SYN_EXEC;

      pg_get_token();
      args = parse_POST_clause();

    }else if (pg_is_ident("wait")) {
      pragmaDir = XMPpragma.WAIT;
      syntax = PragmaSyntax.SYN_EXEC;

      pg_get_token();
      args = parse_WAIT_clause();

    } else if (pg_is_ident("local_alias")) {
      pragmaDir = XMPpragma.LOCAL_ALIAS;
      syntax = PragmaSyntax.SYN_DECL;

      pg_get_token();
      args = parse_LOCAL_ALIAS_clause();
    } else if (pg_is_ident("device")) {
      pg_get_token();
      if (pg_is_ident("replicate")) {
        pragmaDir = XMPpragma.GPU_REPLICATE;
        syntax = PragmaSyntax.SYN_PREFIX;

        pg_get_token();
        args = parse_GPU_REPLICATE_clause();
      } else if (pg_is_ident("replicate_sync")) {
        pragmaDir = XMPpragma.GPU_REPLICATE_SYNC;
        syntax = PragmaSyntax.SYN_EXEC;

        pg_get_token();
        args = parse_GPU_REPLICATE_SYNC_clause();
      } else if (pg_is_ident("reflect")) {
        pragmaDir = XMPpragma.GPU_REFLECT;
        syntax = PragmaSyntax.SYN_EXEC;

        pg_get_token();
        args = parse_REFLECT_clause();
      } else if (pg_is_ident("barrier")) {
        pragmaDir = XMPpragma.GPU_BARRIER;
        syntax = PragmaSyntax.SYN_EXEC;

        pg_get_token();
        args = Xcons.List();
      } else if (pg_is_ident("loop")) {
        pragmaDir = XMPpragma.GPU_LOOP;
        syntax = PragmaSyntax.SYN_PREFIX;

        pg_get_token();
        args = parse_LOOP_clause(true);
      } else {
        error("unknown XcalableMP-GPU directive, '" + pg_tok_buf() + "'");
      }
    } else {
      error("unknown XcalableMP directive, '" + pg_tok_buf() + "'");
    }
 
    if (pg_tok() != 0 || pragmaDir == null) {
      error("extra arguments for XcalableMP directive");
    }

    retSyntax[0] = syntax;
    retArgs[0] = args;

    return pragmaDir;
  }

  private XobjList parse_NODES_clause() throws XmException, XMPexception {
    boolean parseNameList = false;
    XobjList nodesNameList = null;

    // parse <nodes-name>
    if (pg_tok() == PG_IDENT) {
      nodesNameList = Xcons.List(Xcons.String(pg_tok_buf()));
      pg_get_token();
    } else {
      parseNameList = true;
    }

    // parse (<nodes-size>, ...)
    if (pg_tok() != '(')
      error("'(' is expected before <nodes-name>");

    XobjList nodesSizeList = Xcons.List();
    do {
      // parse <nodes-size> := { * | <int-expr> }
      pg_get_token();
      if(pg_tok() == '*') {
        pg_get_token();
        if (pg_tok() == ')') {
          nodesSizeList.add(null);
          break;
        }
        else
          error("'*' can be used only in the last demension");
      }
      else nodesSizeList.add(pg_parse_int_expr());

      if (pg_tok() == ')') break;
      else if (pg_tok() == ',') continue;
      else
        error("')' or ',' is expected after <nodes-size>");
    } while (true);

    // parse { <empty> | =* | =<nodes-ref> }
    XobjList inheritedNodes = null;
    pg_get_token();
    if (pg_tok() == '=') {
      pg_get_token();
      if (pg_tok() == '*') {
        inheritedNodes = Xcons.List(Xcons.IntConstant(XMPnodes.INHERIT_EXEC), null);
        pg_get_token();
      }
      else inheritedNodes = Xcons.List(Xcons.IntConstant(XMPnodes.INHERIT_NODES), parse_ON_REF(false));
    }
    else inheritedNodes = Xcons.List(Xcons.IntConstant(XMPnodes.INHERIT_GLOBAL));

    if (parseNameList) {
      if (pg_tok() == ':') {
        pg_get_token();
        if (pg_tok() == ':') {
          pg_get_token();
          nodesNameList = parse_XMP_obj_name_list("nodes");
        }
      }
    }

    if (nodesNameList == null) {
      error("nodes directive has no <nodes-name>");
    }

    return Xcons.List(nodesNameList, nodesSizeList, inheritedNodes);
  }

  private XobjList parse_TEMPLATE_clause() throws XmException, XMPexception {
    boolean parseNameList = false;
    XobjList templateNameList = null;

    // parse <template-name>
    if (pg_tok() == PG_IDENT) {
      templateNameList = Xcons.List(Xcons.String(pg_tok_buf()));
      pg_get_token();
    } else {
      parseNameList = true;
    }

    // parse (<template-spec>, ...)
    if (pg_tok() != '(') {
      error("'(' is expected after <template-name>");
    }

    XobjList templateSpecList = Xcons.List();
    do {
      // parse <template-spec> := { : | [int-expr :] int-expr }
      pg_get_token();
      if (pg_tok() == ':') {
        pg_get_token();
        if (pg_tok() == ')') {
          templateSpecList.add(null);
          break;
        }
        else
          error("':' can be used only in the last demension");
      }
      else {
        Xobject templateSpec = pg_parse_int_expr();
        if (pg_tok() == ':') {
          pg_get_token();
          templateSpecList.add(Xcons.List(templateSpec, pg_parse_int_expr()));
        }
        else templateSpecList.add(Xcons.List(Xcons.IntConstant(1), templateSpec));
      }

      if (pg_tok() == ')') break;
      else if (pg_tok() == ',') continue;
      else
        error("')' or ',' is expected after <template-spec>");
    } while (true);

    pg_get_token();
    if (parseNameList) {
      if (pg_tok() == ':') {
        pg_get_token();
        if (pg_tok() == ':') {
          pg_get_token();
          templateNameList = parse_XMP_obj_name_list("template");
        }
      }
    }

    if (templateNameList == null) {
      error("template directive has no <template-name>");
    }

    return Xcons.List(templateNameList, templateSpecList);
  }

  private XobjList parse_DISTRIBUTE_clause() throws XMPexception, XmException {
    boolean parseNameList = false;
    XobjList templateNameList = null;

    // parse <template-name>
    if (pg_tok() == PG_IDENT) {
      templateNameList = Xcons.List(Xcons.String(pg_tok_buf()));
      pg_get_token();
    } else {
      parseNameList = true;
    }

    // parse (<dist-format>, ...)
    if (pg_tok() != '(')
      error("'(' is expected after <template-name>");

    XobjList distFormatList = Xcons.List();
    do {
      // FIXME support gblock
      // parse <dist-format> := { * | block | cyclic }
      pg_get_token();
      if (pg_tok() == '*') {
        pg_get_token();
        distFormatList.add(Xcons.List(Xcons.IntConstant(XMPtemplate.DUPLICATION), null));
      } else if (pg_is_ident("block") || pg_is_ident("BLOCK")) {
        pg_get_token();
        distFormatList.add(Xcons.List(Xcons.IntConstant(XMPtemplate.BLOCK), null));
      } else if (pg_is_ident("cyclic") || pg_is_ident("CYCLIC")) {
        pg_get_token();
        if (pg_tok() == '(') {
          pg_get_token();
          Xobject width = pg_parse_int_expr();

          if (pg_tok() != ')') {
            error("')' is needed after <cyclic-width>");
          } else {
            pg_get_token();
            distFormatList.add(Xcons.List(Xcons.IntConstant(XMPtemplate.BLOCK_CYCLIC), width));
          }
        } else {
          distFormatList.add(Xcons.List(Xcons.IntConstant(XMPtemplate.CYCLIC), null));
        }
      } else {
        error("unknown distribution manner");
      }

      if (pg_tok() == ')') break;
      else if (pg_tok() == ',') continue;
      else
        error("')' or ',' is expected after <dist-format>");
    } while (true);

    // parse <onto-clause>
    pg_get_token();
    if (!pg_is_ident("onto"))
      error("distribute directive has no <onto-clause>");

    pg_get_token();
    XobjString nodesName = null;
    if (pg_tok() == PG_IDENT) nodesName = Xcons.String(pg_tok_buf());
    else
      error("<nodes-name> is expected after 'onto'");

    pg_get_token();
    if (parseNameList) {
      if (pg_tok() == ':') {
        pg_get_token();
        if (pg_tok() == ':') {
          pg_get_token();
          templateNameList = parse_XMP_obj_name_list("distribute");
        }
      }
    }

    if (templateNameList == null) {
      error("distribute directive has no <template-name>");
    }

    return Xcons.List(templateNameList, distFormatList, nodesName);
  }

  private XobjList parse_ALIGN_clause() throws XmException, XMPexception {
    boolean parseNameList = false;
    XobjList arrayNameList = null;

    // parse <array-name>
    if (pg_tok() == PG_IDENT) {
      arrayNameList = Xcons.List(Xcons.String(pg_tok_buf()));
      pg_get_token();
    } else {
      parseNameList = true;
    }

    // parse [align-source] ...
    XobjList alignSourceList = Xcons.List();
    if (pg_tok() != '[')
      error("'[' is expected");

    pg_get_token();
    parse_ALIGN_SOURCE(alignSourceList);

    if (pg_tok() != ']')
      error("']' is expected after <align-source>");

    do {
      pg_get_token();
      if (pg_tok() == '[') {
        pg_get_token();
        parse_ALIGN_SOURCE(alignSourceList);

        if (pg_tok() != ']')
          error("']' is expected after <align-source>");
      }
      else break;
    } while (true);

    if (!pg_is_ident("with"))
      error("'with' is expected after ']'");

    // parse <template-name>
    pg_get_token();
    if (pg_tok() != PG_IDENT)
      error("align directive has no <template-name>");

    XobjString templateName = Xcons.String(pg_tok_buf());

    // parse [align-subscript] ...
    pg_get_token();
    if (pg_tok() != '(')
      error("'(' is expected after <template-name>");

    XobjList alignSubscriptList = Xcons.List(Xcons.List(), Xcons.List());
    do {
      pg_get_token();
      parse_ALIGN_SUBSCRIPT(alignSubscriptList);

      if (pg_tok() == ')') break;
      else if (pg_tok() == ',') continue;
      else
        error("')' or ',' is expected after <align-subscript>");
    } while (true);

    pg_get_token();
    if (parseNameList) {
      if (pg_tok() == ':') {
        pg_get_token();
        if (pg_tok() == ':') {
          pg_get_token();
          arrayNameList = parse_XMP_obj_name_list("align");
        }
      }
    }

    if (arrayNameList == null) {
      error("align directive has no <array-name>");
    }

    return Xcons.List(arrayNameList, alignSourceList, templateName, alignSubscriptList);
  }

  private void parse_ALIGN_SOURCE(XobjList alignSourceList) throws XMPexception {
    if (pg_tok() == '*')
      alignSourceList.add(Xcons.String(XMP.ASTERISK));
    else if (pg_tok() == ':')
      alignSourceList.add(Xcons.String(XMP.COLON));
    else if (pg_tok() == PG_IDENT)
      alignSourceList.add(Xcons.String(pg_tok_buf()));
    else
      error("'*' or <scalar-int-variable> is expected for <align-source>");

    pg_get_token();
  }

  private void parse_ALIGN_SUBSCRIPT(XobjList alignSubscriptList) throws XmException, XMPexception {
    if (pg_tok() == '*') {
      alignSubscriptList.left().add(Xcons.String(XMP.ASTERISK));
      alignSubscriptList.right().add(Xcons.IntConstant(0));
      pg_get_token();
      return;
    }
    else if (pg_tok() == ':') {
      alignSubscriptList.left().add(Xcons.String(XMP.COLON));
      alignSubscriptList.right().add(Xcons.IntConstant(0));
      pg_get_token();
      return;
    }
    else if (pg_tok() == PG_IDENT) {
      XobjString var = Xcons.String(pg_tok_buf());
      alignSubscriptList.left().add(var);

      Xobject expr = null;
      pg_get_token();
      if (pg_tok() == '+') {
        pg_get_token();
        expr = pg_parse_int_expr();
      }
      else if (pg_tok() == '-') expr = pg_parse_int_expr();
      else expr = Xcons.IntConstant(0);
      alignSubscriptList.right().add(expr);

      return;
    }
    else
      error("'*' or \"<scalar-int-variable> [+/- <int-expr>]\" is expected for <align-subscript>");
  }

  private XobjList parse_SHADOW_clause() throws XmException, XMPexception {
    boolean parseNameList = false;
    XobjList arrayNameList = null;

    // parse <array-name>
    if (pg_tok() == PG_IDENT) {
      arrayNameList = Xcons.List(Xcons.String(pg_tok_buf()));
      pg_get_token();
    } else {
      parseNameList = true;
    }

    // parse [shadow-width] ...
    XobjList shadowWidthList = Xcons.List();
    if (pg_tok() != '[')
      error("'[' is expected");

    pg_get_token();
    parse_SHADOW_WIDTH(shadowWidthList);

    if (pg_tok() != ']')
      error("']' is expected after <shadow-width>");

    do {
      pg_get_token();
      if (pg_tok() == '[') {
        pg_get_token();
        parse_SHADOW_WIDTH(shadowWidthList);

        if (pg_tok() != ']')
          error("']' is expected after <shadow-width>");
      }
      else break;
    } while (true);

    if (parseNameList) {
      if (pg_tok() == ':') {
        pg_get_token();
        if (pg_tok() == ':') {
          pg_get_token();
          arrayNameList = parse_XMP_obj_name_list("shadow");
        }
      }
    }

    if (arrayNameList == null) {
      error("shadow directive has no <array-name>");
    }

    return Xcons.List(arrayNameList, shadowWidthList);
  }

  private void parse_SHADOW_WIDTH(XobjList shadowWidthList) throws XmException, XMPexception {
    if (pg_tok() == '*') {
      shadowWidthList.add(Xcons.List(Xcons.IntConstant(XMPshadow.SHADOW_FULL), null));

      pg_get_token();
      return;
    } else {
      Xobject width = pg_parse_int_expr();
      if (pg_tok() == ':') {
        pg_get_token();
        shadowWidthList.add(Xcons.List(Xcons.IntConstant(XMPshadow.SHADOW_NORMAL),
                                       Xcons.List(width, pg_parse_int_expr())));
        return;
      }
      else {
        shadowWidthList.add(Xcons.List(Xcons.IntConstant(XMPshadow.SHADOW_NORMAL),
                                       Xcons.List(width, width)));
        return;
      }
    }
  }

  private XobjList parse_TASK_clause() throws XmException, XMPexception {
    Xobject onRef = null;
    if (!pg_is_ident("on"))
      error("task directive has no <on-ref>");

    // parse <on-ref>
    pg_get_token();
    onRef = parse_ON_REF(false);

    Xobject profileClause = null;
    if (pg_is_ident("profile")) {
	profileClause = Xcons.StringConstant("profile");
	pg_get_token();
    }
    
    return Xcons.List(onRef, profileClause);
  }

  private XobjList parse_TASKS_clause() {
    XobjInt nowait = null;
    if (pg_is_ident("nowait")) {
      nowait = Xcons.IntConstant(XMPtask.NOWAIT_TASKS);
      pg_get_token();
    }
    else nowait = null;

    return Xcons.List(nowait);

    // check body in translator: task directive list
  }

  private XobjList parse_LOOP_clause(boolean isDeviceLoop) throws XmException, XMPexception {
    XobjList loopIndexList = null;
    if (pg_tok() == '(') {
      loopIndexList = Xcons.List();
      do {
        pg_get_token();
        if (pg_tok() == PG_IDENT) loopIndexList.add(Xcons.String(pg_tok_buf()));
        else
          error("<loop-index> is not found");

        pg_get_token();
        if (pg_tok() == ',') continue;
        else if (pg_tok() == ')') break;
        else
          error("')' or ',' is expected after <loop-index>");
      } while (true);

      pg_get_token();
    }

    if (!pg_is_ident("on"))
      error("loop directive has no <on-ref>");

    // parse <on-ref>
    pg_get_token();
    XobjList onRef = parse_ON_REF(true);

    // parse [<reduction-ref>], ...
    XobjList reductionRefList = null;
    if (pg_is_ident("reduction")) {
      reductionRefList = Xcons.List();

      pg_get_token();
      reductionRefList.add(parse_REDUCTION_REF());

      do {
        if (pg_is_ident("reduction")) {
          pg_get_token();
          reductionRefList.add(parse_REDUCTION_REF());
        }
        else break;
      } while (true);
    }

    XobjList multicoreClause = null;

    // parse [<gpu-clause>], ...
    if (pg_is_ident("threads")) {
      if (isDeviceLoop) {
        pg_get_token();
        multicoreClause = Xcons.List(Xcons.String("acc"), parse_GPU_clause());
      } else {
        pg_get_token();
        multicoreClause = Xcons.List(Xcons.String("threads"), parse_THREADS_clause());
      }
    }
    
    // parse [profile]                                                                                                   
    Xobject profileClause = null;
    if (pg_is_ident("profile")) {
        profileClause = Xcons.StringConstant("profile");
        pg_get_token();
    }

    return Xcons.List(loopIndexList, onRef, reductionRefList, multicoreClause, profileClause);

    // check body in translator: for loop
  }

  private XobjList parse_REFLECT_clause() throws XmException, XMPexception {
    XobjList arrayNameList = parse_XMP_symbol_list("reflect");

    Xobject profileClause = null;
    if (pg_is_ident("profile")) {
        profileClause = Xcons.StringConstant("profile");
        pg_get_token();
    }

    return Xcons.List(arrayNameList, profileClause);
  }

  private XobjList parse_BARRIER_clause() throws XmException, XMPexception {
    Xobject onRef = null;
    if (pg_is_ident("on")) {
      pg_get_token();
      onRef = parse_ON_REF(false);
    }

    Xobject profileClause = null;
    if (pg_is_ident("profile")) {
	profileClause = Xcons.StringConstant("profile");
	pg_get_token();
    }

    return Xcons.List(onRef, profileClause);
  }

  private XobjList parse_REDUCTION_clause() throws XmException, XMPexception {
    XobjList reductionRef = parse_REDUCTION_REF();
    Xobject onRef = null;

    if (pg_is_ident("on")) {
      pg_get_token();
      onRef = parse_ON_REF(false);
    }

    Xobject profileClause = null;
    if (pg_is_ident("profile")) {
	    profileClause = Xcons.StringConstant("profile");
	    pg_get_token();
	}

    return Xcons.List(reductionRef, onRef, profileClause);
  }

  private XobjList parse_BCAST_clause() throws XmException, XMPexception {
    XobjList varList = parse_XMP_symbol_list("bcast");

    XobjList fromRef = null;
    if (pg_is_ident("from")) {
      pg_get_token();
      fromRef = parse_ON_REF(false);
    }
    else {
      fromRef = null;
    }

    XobjList onRef = null;
    if (pg_is_ident("on")) {
      pg_get_token();
      onRef = parse_ON_REF(false);
    }
    else onRef = null;

    Xobject profileClause = null;
    if (pg_is_ident("profile")) {
        profileClause = Xcons.StringConstant("profile");
        pg_get_token();
    }

    return Xcons.List(varList, fromRef, onRef, profileClause);
  }

  // Fix me
  private XobjList parse_pg_node() throws XmException, XMPexception {
    XobjString objName = Xcons.String(pg_tok_buf());

    pg_get_token();

    if (pg_tok() != '(')
      error("Node syntax Error");

    Xobject nodeNum = pg_parse_expr();
      
    return Xcons.List(objName, nodeNum);
  }

  private XobjList parse_ON_REF(boolean isLoopPragma) throws XmException, XMPexception {
    if (pg_tok() == PG_IDENT) {
      // parse <named-obj-ref>
      XobjString objName = Xcons.String(pg_tok_buf());

      pg_get_token();
      if (pg_tok() != '(') {
        return Xcons.List(objName, null);
      }

      XobjList objSubscriptList = Xcons.List();
      do {
        pg_get_token();
        parse_OBJ_SUBSCRIPT(objSubscriptList, isLoopPragma);

        if (pg_tok() == ')') {
          break;
        } else if (pg_tok() == ',') {
          continue;
        } else {
          error("')' or ',' is expected after <nodes/template-subscript>");
        }
      } while (true);

      pg_get_token();
      return Xcons.List(objName, objSubscriptList);
    } else {
      if (isLoopPragma) {
        error("<node-number-ref> cannot be used in for directive");
      }

      // parse <node-number-ref>
      XobjList nodeNumberRef = null;
      if (pg_tok() == '(') {
        pg_get_token();
        nodeNumberRef = pg_parse_int_triplet(false);

        if (pg_tok() == ')') {
          pg_get_token();
        } else {
          error("')' is expected after <triplet>");
        }
      } else {
        Xobject nodeNumber = pg_parse_int_expr();
        nodeNumberRef = Xcons.List(nodeNumber, nodeNumber, Xcons.IntConstant(1));
      }

      return Xcons.List((Xobject)null, nodeNumberRef);
    }
  }

  private void parse_OBJ_SUBSCRIPT(XobjList nodesSubscriptList, boolean isLoopPragma) throws XmException, XMPexception {
    if (isLoopPragma) {
      if (pg_tok() == '*') {
        nodesSubscriptList.add(Xcons.String(XMP.ASTERISK));
      } else if (pg_tok() == ':') {
        nodesSubscriptList.add(Xcons.String(XMP.COLON));
      } else {
        if (pg_tok() == PG_IDENT) {
          nodesSubscriptList.add(Xcons.String(pg_tok_buf()));
        } else {
          error("syntax error on <nodes/template-subscript> in for directive");
        }
      }

      pg_get_token();
      return;
    } else {
      if (pg_tok() == '*') {
        nodesSubscriptList.add(null);
        pg_get_token();
        return;
      } else {
        nodesSubscriptList.add(pg_parse_int_triplet(true));
        return;
      }
    }
  }

  private XobjList parse_REDUCTION_REF() throws XMPexception {
    // 'reduction' is already parsed
    if (pg_tok() != '(')
      error("'(' is expected after 'reduction'");

    // parse <reduction-kind>
    pg_get_token();
    XobjInt reductionKind = get_REDUCTION_KIND();

    pg_get_token();
    if (pg_tok() != ':')
      error("':' is expected after <reduction-kind>");

    // parse <reduction-spec>, ...
    XobjList reductionSpecList = Xcons.List();
    do {
      pg_get_token();
      reductionSpecList.add(parse_REDUCTION_SPEC(reductionKind.getInt()));

      if (pg_tok() == ')') break;
      else if (pg_tok() == ',') continue;
      else
        error("')' or ',' is expected after <reduction-spec>");
    } while (true);

    pg_get_token();
    return Xcons.List(reductionKind, reductionSpecList);
  }

  private XobjInt get_REDUCTION_KIND() throws XMPexception {
    // FIXME some opearations are not implemented yet
    switch (pg_tok()) {
      case '+':
        return Xcons.IntConstant(XMPcollective.REDUCE_SUM);
      case '*':
        return Xcons.IntConstant(XMPcollective.REDUCE_PROD);
      case '&':
        return Xcons.IntConstant(XMPcollective.REDUCE_BAND);
      case PG_ANDAND:
        return Xcons.IntConstant(XMPcollective.REDUCE_LAND);
      case '|':
        return Xcons.IntConstant(XMPcollective.REDUCE_BOR);
      case PG_OROR:
        return Xcons.IntConstant(XMPcollective.REDUCE_LOR);
      case '^':
        return Xcons.IntConstant(XMPcollective.REDUCE_BXOR);
      case PG_IDENT:
        {
          if (pg_is_ident("sum") || pg_is_ident("SUM"))
            return Xcons.IntConstant(XMPcollective.REDUCE_SUM);
          else if (pg_is_ident("prod") || pg_is_ident("PROD"))
            return Xcons.IntConstant(XMPcollective.REDUCE_PROD);
          else if (pg_is_ident("band") || pg_is_ident("BAND"))
            return Xcons.IntConstant(XMPcollective.REDUCE_BAND);
          else if (pg_is_ident("land") || pg_is_ident("LAND"))
            return Xcons.IntConstant(XMPcollective.REDUCE_LAND);
          else if (pg_is_ident("bor") || pg_is_ident("BOR"))
            return Xcons.IntConstant(XMPcollective.REDUCE_BOR);
          else if (pg_is_ident("lor") || pg_is_ident("LOR"))
            return Xcons.IntConstant(XMPcollective.REDUCE_LOR);
          else if (pg_is_ident("bxor") || pg_is_ident("BXOR"))
            return Xcons.IntConstant(XMPcollective.REDUCE_BXOR);
          else if (pg_is_ident("lxor") || pg_is_ident("LXOR"))
            return Xcons.IntConstant(XMPcollective.REDUCE_LXOR);
          else if (pg_is_ident("max") || pg_is_ident("MAX"))
            return Xcons.IntConstant(XMPcollective.REDUCE_MAX);
          else if (pg_is_ident("min") || pg_is_ident("MIN"))
            return Xcons.IntConstant(XMPcollective.REDUCE_MIN);
          else if (pg_is_ident("firstmax") || pg_is_ident("FIRSTMAX"))
            return Xcons.IntConstant(XMPcollective.REDUCE_FIRSTMAX);
          else if (pg_is_ident("firstmin") || pg_is_ident("FIRSTMIN"))
            return Xcons.IntConstant(XMPcollective.REDUCE_FIRSTMIN);
          else if (pg_is_ident("lastmax") || pg_is_ident("LASTMAX"))
            return Xcons.IntConstant(XMPcollective.REDUCE_LASTMAX);
          else if (pg_is_ident("lastmin") || pg_is_ident("LASTMIN"))
            return Xcons.IntConstant(XMPcollective.REDUCE_LASTMIN);
        }
      default:
        error("'" + pg_tok_buf() +  "' is not allowed for <reduction-spec>");
        // XXX never reach here
        return null;
    }
  }

  private XobjList parse_REDUCTION_SPEC(int reductionKind) throws XMPexception {
    XobjList reductionSpec = Xcons.List();
    if (pg_tok() == PG_IDENT){
      reductionSpec.add(Xcons.String(pg_tok_buf()));
    }
    else if(pg_tok() == '*'){
      pg_get_token();
      reductionSpec.add(Xcons.String(pg_tok_buf()));
    }
    else
      error("syntax error on <reduction-spec>");

    pg_get_token();
    switch (reductionKind) {
      case XMPcollective.REDUCE_SUM:
      case XMPcollective.REDUCE_PROD:
      case XMPcollective.REDUCE_BAND:
      case XMPcollective.REDUCE_LAND:
      case XMPcollective.REDUCE_BOR:
      case XMPcollective.REDUCE_LOR:
      case XMPcollective.REDUCE_BXOR:
      case XMPcollective.REDUCE_LXOR:
      case XMPcollective.REDUCE_MAX:
      case XMPcollective.REDUCE_MIN:
        reductionSpec.add(null);
        break;
      case XMPcollective.REDUCE_FIRSTMAX:
      case XMPcollective.REDUCE_FIRSTMIN:
      case XMPcollective.REDUCE_LASTMAX:
      case XMPcollective.REDUCE_LASTMIN:
        {
          if (pg_tok() != '/')
            error("'/' is expected after <reduction-variable>");

          XobjList locationVariables = Xcons.List();
          do {
            pg_get_token();
            if (pg_tok() == PG_IDENT)
              locationVariables.add(Xcons.String(pg_tok_buf()));
	    else if (pg_tok() == '*'){  // Pointer Reference
	      pg_get_token();
	      locationVariables.add(Xcons.String(pg_tok_buf()));
	    }
            else
              error("syntax error on <location-variable>");

            pg_get_token();
            if (pg_tok() == '/') break;
            else if (pg_tok() == ',') continue;
            else
              error("'/' or ',' is expected after <reduction-spec>");
          } while (true);

          reductionSpec.add(locationVariables);
          pg_get_token();
        }
        break;
      default:
        error("unknown reduce operation on <reduction-spec>");
    }

    return reductionSpec;
  }

  private XobjList parse_GMOVE_clause() throws XMPexception {
    XobjInt gmoveClause = null;
    if (pg_is_ident("in")) {
      gmoveClause = Xcons.IntConstant(XMPcollective.GMOVE_IN);
      pg_get_token();
    }
    else if (pg_is_ident("out")) {
      gmoveClause = Xcons.IntConstant(XMPcollective.GMOVE_OUT);
      pg_get_token();
    }
    else gmoveClause = Xcons.IntConstant(XMPcollective.GMOVE_NORMAL);

    Xobject profileClause = null;
    if (pg_is_ident("profile")) {
        profileClause = Xcons.StringConstant("profile");
        pg_get_token();
    }

    return Xcons.List(gmoveClause, profileClause);
  }

  private XobjList parse_POST_clause() throws XmException, XMPexception {
    if (pg_tok() != '(')
      error("'(' is expected before <nodes-name, tag>");

    pg_get_token();

    Xobject onRef = parse_ON_REF(false);
    pg_get_token();
    Xobject tag = pg_parse_expr();

    if (pg_tok() != ')') {
      error("')' is expected after <nodes-name, tag>");
    }
    pg_get_token();
    return Xcons.List(onRef, tag);
  }

  private XobjList parse_WAIT_clause() throws XmException, XMPexception {
    if (pg_tok() != '('){
      pg_get_token();
      return Xcons.List(Xcons.IntConstant(0));  // 0 is a number of args
    }
    else{
      pg_get_token(); 
      XobjList nodeName = parse_pg_node();
      if (pg_tok() == ','){
	pg_get_token();
        Xobject tag = pg_parse_expr();
        pg_get_token();
        return Xcons.List(Xcons.IntConstant(2), nodeName, tag);
      }
      else{  // if(pg_tok() == ')')
	pg_get_token();
	return Xcons.List(Xcons.IntConstant(1), nodeName);
      }
    }
  }

  private XobjList parse_COARRAY_clause() throws XmException, XMPexception {
    XobjList coarrayNameList = parse_XMP_obj_name_list("coarray");

    if (pg_tok() != ':') {
      error("':' is expected before <coarray-dimensions>");
    }

    boolean parsedLastDim = false;
    XobjList coarrayDims = Xcons.List();

    pg_get_token();
    if (pg_tok() != '[') {
      error("'[' is expected before <coarray-dim>");
    }

    pg_get_token();
    if (pg_tok() == '*') {
      parsedLastDim = true;
      coarrayDims.add(null);
      pg_get_token();
    } else {
      coarrayDims.add(pg_parse_expr());
    }

    if (pg_tok() != ']') {
      error("']' is expected after <coarray-dim>");
    }
    
    while (true) {
      pg_get_token();
      if (pg_tok() == '[') {
        if (parsedLastDim) {
          error("'*' in <coarray-dimension> is used in a wrong place");
        }

        pg_get_token();
        if (pg_tok() == '*') {
          parsedLastDim = true;
          coarrayDims.add(null);
          pg_get_token();
        } else {
          coarrayDims.add(pg_parse_expr());
        }

        if (pg_tok() != ']') {
          error("']' is expected after <coarray-dim>");
        }
      } else {
        break;
      }
    }

    return Xcons.List(coarrayNameList, coarrayDims);
  }

  private XobjList parse_LOCAL_ALIAS_clause() throws XMPexception {
    XobjString localArrayName = null;
    if (pg_tok() == PG_IDENT) {
      localArrayName = Xcons.String(pg_tok_buf());
    } else {
      error("<local-array-name> is not indicated");
    }

    pg_get_token();
    if (pg_tok() != '=') {
      error("'=>' is expected after <local-array-name>");
    }

    pg_get_token();
    if (pg_tok() != '>') {
      error("'=>' is expected after <local-array-name>");
    }

    XobjString globalArrayName = null;
    pg_get_token();
    if (pg_tok() == PG_IDENT) {
      globalArrayName = Xcons.String(pg_tok_buf());
    } else {
      error("<local-array-name> is not indicated");
    }

    pg_get_token();
    return Xcons.List(localArrayName, globalArrayName);
  }

  private XobjList parse_GPU_clause() throws XmException, XMPexception {
    XobjList args = Xcons.List();

    while (pg_tok() == PG_IDENT) {
      if (pg_is_ident("private")) {
        pg_get_token();
        XobjList v = parse_XMP_symbol_list("gpu private clause");
        args.add(xmp_pg_list(XMPpragma.GPU_PRIVATE, v));
      } else if (pg_is_ident("firstprivate")) {
        pg_get_token();
        XobjList v = parse_XMP_symbol_list("gpu firstprivate clause");
        args.add(xmp_pg_list(XMPpragma.GPU_FIRSTPRIVATE, v));
      } else if (pg_is_ident("map_threads")) {
        pg_get_token();
        if (pg_tok() != '(') {
          throw new XMPexception("'(' is expected after 'map_threads'");
        }

        XobjList threadX = null;
        XobjList threadY = null;
        XobjList threadZ = null;

        pg_get_token();
        if (pg_tok() == PG_IDENT) {
          threadX = Xcons.List(Xcons.String(pg_tok_buf()));
        } else {
          throw new XMPexception("a loop variable is needed in 'map_threads'");
        }

        pg_get_token();
	if (pg_tok() == ':') {
          pg_get_token();
          threadX.add(pg_parse_expr());
        } else {
          throw new XMPexception("':' is needed after a loop variable in 'map_threads'");
        }

        if (pg_tok() == ',') {
          pg_get_token();
          if (pg_tok() == PG_IDENT) {
            threadY = Xcons.List(Xcons.String(pg_tok_buf()));
          } else {
            throw new XMPexception("a loop variable is needed in 'map_threads'");
          }

          pg_get_token();
          if (pg_tok() == ':') {
            pg_get_token();
            threadY.add(pg_parse_expr());
          } else {
            throw new XMPexception("':' is needed after a loop variable in 'map_threads'");
          }

          if (pg_tok() == ',') {
            pg_get_token();
            if (pg_tok() == PG_IDENT) {
              threadZ = Xcons.List(Xcons.String(pg_tok_buf()));
            } else {
              throw new XMPexception("a loop variable is needed in 'map_threads'");
            }

            pg_get_token();
            if (pg_tok() == ':') {
              pg_get_token();
              threadZ.add(pg_parse_expr());
            } else {
              throw new XMPexception("':' is needed after a loop variable in 'map_threads'");
            }
          }
        }

        args.add(xmp_pg_list(XMPpragma.GPU_MAP_THREADS, Xcons.List(threadX, threadY, threadZ)));

        if (pg_tok() != ')') {
          throw new XMPexception("')' is expected after <map_threads> clause");
        }

        pg_get_token();
      } else {
        throw new XMPexception("unknown acc clause ="+pg_tok_buf());
      }
    }

    return args;
  }

  private XobjList parse_THREADS_clause() throws XmException, XMPexception {
    XobjList args = Xcons.List();

    while (pg_tok() == PG_IDENT) {
      if (pg_is_ident("private")) {
        pg_get_token();
        XobjList v = parse_THREADS_namelist_as_symbol();
        args.add(omp_pg_list(OMPpragma.DATA_PRIVATE, v));
      }
      else if (pg_is_ident("firstprivate")) {
        pg_get_token();
        XobjList v = parse_THREADS_namelist_as_symbol();
        args.add(omp_pg_list(OMPpragma.DATA_FIRSTPRIVATE, v));
      }
      else if (pg_is_ident("lastprivate")) {
        pg_get_token();
        XobjList v = parse_THREADS_namelist_as_symbol();
        args.add(omp_pg_list(OMPpragma.DATA_LASTPRIVATE, v));
      }
      else if (pg_is_ident("num_threads")) {
        pg_get_token();
        if (pg_tok() != '(') {
          throw new XMPexception("'(' is expected after 'num_threads'");
        }

        pg_get_token();
        Xobject v = pg_parse_expr();
        args.add(omp_pg_list(OMPpragma.DIR_NUM_THREADS, v));

        if (pg_tok() != ')') {
          throw new XMPexception("')' is expected after <num_threads> clause");
        }

        pg_get_token();
      }
      else if (pg_is_ident("if")) {
        pg_get_token();
        if (pg_tok() != '(') {
          throw new XMPexception("'(' is expected after 'if'");
        }

        pg_get_token();
        Xobject v = pg_parse_expr();
        args.add(omp_pg_list(OMPpragma.DIR_IF, v));

        if (pg_tok() != ')') {
          throw new XMPexception("')' is expected after <if> clause");
        }

        pg_get_token();
      }
      else {
        throw new XMPexception("unknown threads clause");
      }
    }

    return args;
  }

  private XobjList omp_pg_list(OMPpragma pg, Xobject args) {
    return Xcons.List(Xcode.LIST, Xcons.String(pg.toString()), args);
  }

  private XobjList xmp_pg_list(XMPpragma pg, Xobject args) {
    return Xcons.List(Xcode.LIST, Xcons.String(pg.toString()), args);
  }

  private XobjList parse_THREADS_namelist_as_symbol() throws XMPexception {
    return parse_THREADS_namelist(true);
  }

  private XobjList parse_THREADS_namelist_as_not_symbol() throws XMPexception {
    return parse_THREADS_namelist(false);
  }

  private XobjList parse_THREADS_namelist(boolean asSymbol) throws XMPexception {
    XobjList args = Xcons.List();

    if (pg_tok() != '(') {
      error("threads clause requires name list");
    }

    while (true) {
      pg_get_token();

      if (pg_tok() != PG_IDENT) {
        error("empty name list in threads clause");
      }

      if (asSymbol) {
        XmSymbol sym = XmSymbolUtil.lookupSymbol(_lexer.getContext(), pg_tok_buf());
        if (sym == null) {
          error("undefined identifier in threads clause : " + pg_tok_buf());
        }
        else if (!sym.isIdent() &&
                 !sym.getSclass().equals(StorageClass.FCOMMON_NAME)) {
          error("bad identifier in threads clause : " + pg_tok_buf());
        }
      }
      args.add(Xcons.Symbol(Xcode.IDENT, pg_tok_buf()));

      pg_get_token();
      if (pg_tok() == ')') {
        pg_get_token();
        return args;
      }
      else if(pg_tok() != ',') {
        break;
      }
    }

    error("syntax error in threads clause");

    // XXX never reach here
    return null;
  }

  private XobjList parse_GPU_REPLICATE_clause() throws XMPexception {
    XobjList varList = parse_XMP_symbol_list("device replicate");
    return Xcons.List(varList);
  }

  private XobjList parse_GPU_REPLICATE_SYNC_clause() throws XMPexception {
    XobjList clauseList = Xcons.List();

    while (true) {
      if (pg_is_ident("in") || pg_is_ident("out")) {
        XobjString clauseName = Xcons.String(pg_tok_buf());

        pg_get_token();
        XobjList varList = parse_XMP_symbol_list("device replicate_sync");

        clauseList.add(Xcons.List(clauseName, varList));
      } else {
        throw new XMPexception("'in' or 'out' clause is required in device replicate_sync directive");
      }

      if (!(pg_is_ident("in") || pg_is_ident("out"))) {
        break;
      }
    }

    return clauseList;
  }

  private XobjList parse_GPU_LOOP_clause() throws XmException, XMPexception {
    XobjList loopVarList = null;
    if (pg_tok() == '(') {
      loopVarList = parse_XMP_symbol_list("device loop");
    }

    // FIXME needs reduction clause
    XobjList gpuClause = parse_GPU_clause();

    // XMP loop gpu clause format
    return Xcons.List(loopVarList, null, null, Xcons.List(Xcons.String("gpu"), gpuClause));
  }

  private XobjList parse_XMP_symbol_list(String name) throws XMPexception {
    XobjList varList = Xcons.List();

    if (pg_tok() != '(') {
      throw new XMPexception("'(' is expected before " + name + " <variable> list");
    }

    do {
      pg_get_token();
      if (pg_tok() == PG_IDENT) {
	varList.add(Xcons.String(pg_tok_buf()));
      }
      else if (pg_tok() == '*'){  // Pointer Reference 
	pg_get_token();
	varList.add(Xcons.String(pg_tok_buf()));
      }
      else {
        throw new XMPexception("<variable> for " + name + " is expected");
      }

      pg_get_token();
      if (pg_tok() == ',') {
        continue;
      }
      else if (pg_tok() == ')') {
        break;
      }
      else {
        throw new XMPexception("',' or ')' is expected after " + name + " <variable> list");
      }
    } while (true);

    pg_get_token();
    return varList;
  }

  private XobjList parse_XMP_obj_name_list(String name) throws XMPexception {
    XobjList varList = Xcons.List();

    do {
      if (pg_tok() == PG_IDENT) {
        varList.add(Xcons.String(pg_tok_buf()));
      } else {
        throw new XMPexception("<variable> for " + name + " is expected");
      }

      pg_get_token();
      if (pg_tok() == ',') {
        pg_get_token();
        continue;
      } else {
        return varList;
      }
    } while (true);
  }
}
