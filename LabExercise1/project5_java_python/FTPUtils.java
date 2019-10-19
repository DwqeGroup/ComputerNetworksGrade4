import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.SocketException;

import org.apache.commons.net.ftp.FTPClient;
import org.apache.commons.net.ftp.FTPFile;
import org.apache.commons.net.ftp.FTPReply;
import org.apache.log4j.Logger;

/**
 * 简单操作FTP工具类 ,此工具类支持中文文件名，不支持中文目录
 * 如果需要支持中文目录，需要 new String(path.getBytes("UTF-8"),"ISO-8859-1") 对目录进行转码
 * @author WZH
 * 
 */
public class FTPUtils {

    private static Logger logger = Logger.getLogger(FTPUtils.class);

    /**
     * 获取FTPClient对象
     * @param ftpHost 服务器IP
     * @param ftpPort 服务器端口号
     * @param ftpUserName 用户名
     * @param ftpPassword 密码
     * @return FTPClient
     */
    public FTPClient getFTPClient(String ftpHost, int ftpPort,
            String ftpUserName, String ftpPassword) {

        FTPClient ftp = null;
        try {
            ftp = new FTPClient();
            // 连接FPT服务器,设置IP及端口
            ftp.connect(ftpHost, ftpPort);
            // 设置用户名和密码
            ftp.login(ftpUserName, ftpPassword);
            // 设置连接超时时间,5000毫秒
            ftp.setConnectTimeout(50000);
            // 设置中文编码集，防止中文乱码
            ftp.setControlEncoding("UTF-8");
            if (!FTPReply.isPositiveCompletion(ftp.getReplyCode())) {
                logger.info("未连接到FTP，用户名或密码错误");
                ftp.disconnect();
            } else {
                logger.info("FTP连接成功");
                //System.out.println("连接成功");
            }

        } catch (SocketException e) {
            e.printStackTrace();
            logger.info("FTP的IP地址可能错误，请正确配置");
        } catch (IOException e) {
            e.printStackTrace();
            logger.info("FTP的端口错误,请正确配置");
        }
        return ftp;
    }
    
    /**
     * 关闭FTP方法
     * @param ftp
     * @return
     */
    public boolean closeFTP(FTPClient ftp){
        
        try {
            ftp.logout();
        } catch (Exception e) {
            logger.error("FTP关闭失败");
        }finally{
            if (ftp.isConnected()) {
                try {
                    ftp.disconnect();
                } catch (IOException ioe) {
                    logger.error("FTP关闭失败");
                }
            }
        }
        
        return false;
        
    }
    
    
    /**
     * 下载FTP下指定文件
     * @param ftp FTPClient对象
     * @param filePath FTP文件路径
     * @param fileName 文件名
     * @param downPath 下载保存的目录
     * @return
     */
    public boolean downLoadFTP(FTPClient ftp, String filePath, String fileName,
            String downPath) {
        // 默认失败
        boolean flag = false;

        try {
            // 跳转到文件目录
            ftp.changeWorkingDirectory(filePath);
            // 获取目录下文件集合
            ftp.enterLocalPassiveMode();
            FTPFile[] files = ftp.listFiles();
            //System.out.println("start dowloading");
            for (FTPFile file : files) {
                // 取得指定文件并下载
                if (file.getName().equals(fileName)) {
                    File downFile = new File(downPath + File.separator
                            + file.getName());
                    System.out.println(downPath + File.separator
                            + file.getName());
                    OutputStream out = new FileOutputStream(downFile);
                    // 绑定输出流下载文件,需要设置编码集，不然可能出现文件为空的情况
                    flag = ftp.retrieveFile(new String(file.getName().getBytes("UTF-8"),"ISO-8859-1"), out);
                    // 下载成功删除文件,看项目需求
                    // ftp.deleteFile(new String(fileName.getBytes("UTF-8"),"ISO-8859-1"));
                    out.flush();
                    out.close();
                    if(flag){
                        logger.info("下载成功");
                    }else{
                        logger.error("下载失败");
                    }
                }
            }
            //System.out.println("downloading end");

        } catch (Exception e) {
            logger.error("下载失败");
        } 

        return flag;
    }

    /**
     * FTP文件上传工具类
     * @param ftp
     * @param filePath
     * @param ftpPath
     * @return
     */
    public boolean uploadFile(FTPClient ftp,String filePath,String ftpPath){
        boolean flag = false;
        InputStream in = null;
        try {
         // 设置PassiveMode传输  
            ftp.enterLocalPassiveMode(); 
            //设置二进制传输，使用BINARY_FILE_TYPE，ASC容易造成文件损坏
            ftp.setFileType(FTPClient.BINARY_FILE_TYPE);
            //判断FPT目标文件夹时候存在不存在则创建
            if(!ftp.changeWorkingDirectory(ftpPath)){
                ftp.makeDirectory(ftpPath);
            }
            //跳转目标目录
            ftp.changeWorkingDirectory(ftpPath);
            
            //上传文件
            File file = new File(filePath);
            in = new FileInputStream(file);
            String tempName = ftpPath+File.separator+file.getName();
            //System.out.println(ftpPath+File.separator+file.getName());
            flag = ftp.storeFile(new String (tempName.getBytes("UTF-8"),"ISO-8859-1"),in);
            if(flag){
                logger.info("上传成功");
            }else{
                logger.error("上传失败");
            }
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("上传失败");
        }finally{
            try {
                in.close();
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        return flag;
    }   
    
    public static void main(String[] args) {
        FTPUtils test = new FTPUtils();
        FTPClient ftp = test.getFTPClient("192.168.43.66", 21, "123","123");
        //test.downLoadFTP(ftp, "/file", "你好.jpg", "C:\\下载");
        //test.copyFile(ftp, "/file", "/txt/temp", "你好.txt");
        //test.uploadFile(ftp, "C:\\下载\\你好.jpg", "/");
        //test.moveFile(ftp, "/file", "/txt/temp");
        //test.deleteByFolder(ftp, "/txt");
        test.downLoadFTP(ftp, "/","test.txt", "C:\\Users\\tc\\Downloads");
        test.uploadFile(ftp, "C:\\Users\\tc\\Downloads\\upload_test.txt", "/");
        test.closeFTP(ftp);
        System.exit(0);
    }
}