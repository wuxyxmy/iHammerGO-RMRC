/*******************************************************************************
 * File name: protocol.js
 * Created on: 2018/05/30 11:27:39
 * Author: Wuxy
 * 本地服务器通讯协议
 * 
 * History:
 *  v1.2
 *    2019/06/30 整理调试输出格式
 *    2019/06/29 增加了一个临时方法 [Server.load_info(cmd, jqTarget)]
 *  v1.1
 *    2018/06/12
 *  v1.0
 *    2018/06/05 修改 Protocol 为 Server
 *    2018/06/03 创建 Protocol 类，添加之前的代码
*******************************************************************************/
(function (global, factory) {

    factory(global);
  
  }(window, function (window) {
    "use strict";
    const $ = window.jQuery;

    var Server = function () {
      return Server.__init__(arguments);
    };
    /******************* Static Constant **************************************/
    Server.COMMAND_PORT = 8090;
    Server.RETRY_COUNT  = 3;
    
    /******************* Constructor ******************************************/
    Server.__init__ = function (args) {
      // empty, no arguments
    } 
    
    /******************* Static Functions *************************************/
    
    Server.prototype = {
    /******************* Member Variables *************************************/
      hostname : null,
      port     : null,

    /******************* Member Functions *************************************/
      execute : function (command) {
        $.ajax({
          type : "GET",
          url : "command",
          data : { cmd : command },

          timeout : 1200,

          success : function (data, status) {
            console.log("AJAX Done: " + status + "\nResponse: " + data);
          },

          error : function (jqXHR, status) {
            console.log("AJAX Fail! \n[Status]: " + status);
          }
        });
      },

      load : function (what, jqTarget) {
        $.ajax({
          type : "GET",
          url : "load",
          data : { what : what },

          timeout : 1200,

          success : function (data, status) {
            jqTarget.html(data);
          },

          error : function (jqXHR, status) {
            console.log("AJAX Fail! \n[Status]: " + status);
          }
        });
      },

      load_info : function (cmd, jqTarget) {
        $.ajax({
          type : "GET",
          url : "info",
          data : { cmd : cmd },

          timeout : 1200,

          success : function (data, status) {
            jqTarget.html(data);
          },

          error : function (jqXHR, status) {
            console.log("AJAX Fail! \n[Status]: " + status);
          }
        });
      },

      barcodeReaderControl : function (enable, method) {
        $.ajax({
          type : "GET",
          url : "/barcode_reader",
          data : { enable : enable ? 1 : 0,
                   method : method || "" },

          timeout : 1200,

          success : function (data, status) {
            console.log("AJAX Done: " + status + "\nResponse: " + data);
          },

          error : function (jqXHR, status) {
            console.log("AJAX Fail! \n[Status]: " + status);
          }
        });
      }
    };
    
    window.Server = Server;
  })
);

/******************************************************************************/
